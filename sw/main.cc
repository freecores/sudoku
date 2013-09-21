#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <time.h>
#include <limits.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include "driver.h"
#include "counters.h"

#define USE_DIV_TABLE 1
const int divTable[9] = {0,0,0,3,3,3,6,6,6};

double timestamp()
{
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

int32_t sudoku(uint32_t *board, uint32_t *os);

int u32_cmp(const void* a, const void *b)
{
  uint32_t x = *((uint32_t*)a);
  uint32_t y = *((uint32_t*)b);
  return (x<y);
}

inline uint32_t ln2(uint32_t x)
{
  uint32_t y = 1;
  while(x > 1)
    {
      y++;
      x = x>>1;
    }
  return y;
}

inline uint32_t one_set(uint32_t x)
{
  /* all ones if pow2, otherwise 0 */
  uint32_t pow2 = (x & (x-1));
  uint32_t m = (pow2 == 0);
  return ((~m) + 1) & x;
}

inline uint32_t find_first_set(uint32_t x)
{
#ifdef __GNUC__
  return __builtin_ctz(x);
#else
  /* copied from the sparc v9 reference manual */
  return count_ones(x ^ (~(-x))) - 1;
#endif
}

inline uint32_t count_ones(uint32_t x)
{
#ifdef __GNUC__
  return __builtin_popcount(x);
#else
  uint32_t y = 0;
  for(y=0;x!=0;y++)
    {
      x &= (x-1);
    }
  return y;
#endif
}

inline uint32_t isPow2(uint32_t x)
{
  uint32_t pow2 = (x & (x-1));
  return (pow2 == 0);
}

int32_t check_correct(uint32_t *board, uint32_t *unsolved_pieces)
{
  int32_t i,j;
  int32_t ii,jj;
  int32_t si,sj;
  int32_t tmp;

  *unsolved_pieces = 0;
  int32_t violated = 0;

  uint32_t counts[81];
  for(i=0;i < 81; i++)
    {
      counts[i] = count_ones(board[i]);
      if(counts[i]!=1)
	{
	  *unsolved_pieces = 1;
	  return 0;
	}
    }
  

  /* for each row */
  for(i=0;i<9;i++)
    {
      uint32_t sums[9] = {0};
      for(j=0;j<9;j++)
	{
	  if(counts[i*9 +j] == 1)
	    {
	      tmp =ln2(board[i*9+j])-1;
	      sums[tmp]++;
	      if(sums[tmp] > 1)
		{
		  //char buf[80];
		  //sprintf_binary(board[i*9+j],buf,80);
		  //printf("violated row %d, sums[%d]=%d, board = %s\n", i, tmp, sums[tmp], buf);
		  //print_board(board);
		  violated = 1;
		  goto done;
		}
	    }
	}
    }
  /* for each column */

   for(j=0;j<9;j++)
   {
     uint32_t sums[9] = {0};
     for(i=0;i<9;i++)
     {
       if(counts[i*9 +j] == 1)
	 {
	   tmp =ln2(board[i*9+j])-1;
	   sums[tmp]++;
	   if(sums[tmp] > 1)
	     {
	       violated = 1;
	       goto done;
	       //printf("violated column %d, sums[%d]=%d\n", i, tmp, sums[tmp]);
	       //return 0;
	     }
	 }
     }
   }

   for(i=0;i<9;i++)
     {
#ifdef USE_DIV_TABLE
       si = divTable[i];
#else
       si = 3*(i/3);
#endif
       for(j=0;j<9;j++)
	 {
#ifdef USE_DIV_TABLE
	   sj = 3*(j/3);
#else
	   sj = divTable[j];
#endif
	   uint32_t sums[9] = {0};
	   for(ii=si;ii<(si+3);ii++)
	     {
	       for(jj=sj;jj<(sj+3);jj++)
		 {
		   if(counts[ii*9 +jj] == 1)
		     {
		       tmp =ln2(board[ii*9+jj])-1;
		       sums[tmp]++;
		       if(sums[tmp] > 1)
			 {
			   violated = 1;
			   goto done;
			 }
		     }
		 }
	     }
	 }
     }

done:
   return (violated == 0);
}



void print_board(uint32_t *board)
{
  int32_t i,j;
  char buf[80] = {0};
  for(i=0;i<9;i++)
    {
      for(j=0;j<9;j++)
	{
	  /* sprintf_binary(board[i*9+j], buf, 80);
	   * printf("%s ", buf); */
	  printf("%d ", ln2(board[i*9+j]));
	}
      printf("\n");
    }
  printf("\n");
}


int32_t hw_solve(Driver *d, uint32_t *board)
{
  uint32_t rc=0;
  d->reset();
  const uint32_t w_bit = 1 << 31;

  /*
  if(init == 0)
    {
      memset(&cCnt,0,sizeof(cCnt));
      initTicks(cCnt);
      init = 1;
    }
  */
  //uint64_t c0 = getTicks(cCnt);
  for(int i = 0; i < 81; i++)
    {
      uint32_t addr = i;
      uint32_t data = board[i];
      uint32_t v = w_bit | (data << 7) | addr;
      d->write(1, v);
    }
  //c0 = getTicks(cCnt) - c0;
  //printf("%llu cycles to write data to FPGA\n", c0);

  /* only pulsed for one cycle */
  d->write(7,1);

  do
    {
      rc = d->read(0) & 0x7;
    } 
  while(rc ==0);
   
  //c0 = getTicks(cCnt);
  for(int i = 0; i < 81; i++)
    {
      uint32_t addr = i;
      d->write(1, addr);
      uint32_t v = d->read(2);
      board[i] = v;
    }
  //c0 = getTicks(cCnt) - c0;
  //printf("%llu cycles to read data from FPGA\n", c0);
  
  return (rc == 0x1);
}

int main(int argc, char **argv)
{
  uint32_t *cpuBoard, *hwBoard;
  uint32_t *os; 
  uint64_t c0,c1,i0;
  int i,j,rc;
  uint32_t **puzzles;
  uint32_t **cpyPuzzles;
  uint32_t nPuzzles = 0;
  uint32_t *speedUps = 0;
  int c;  

  hwCounter_t cCnt,iCnt;
  memset(&cCnt,0,sizeof(cCnt));
  memset(&iCnt,0,sizeof(iCnt));
  initTicks(cCnt);
  initInsns(iCnt);

  Driver *d = new Driver(0x79100000);

  FILE *fp = 0;

  if(argc < 2)
    return -1;
  

  fp = fopen(argv[1], "r");
  assert(fp);
  


  while((c = fgetc(fp)) != EOF)
    {
      if(c == '\n')
        {
          nPuzzles++;
        }
    }
  rewind(fp);
  puzzles = (uint32_t**)malloc(sizeof(uint32_t*)*nPuzzles);
  cpyPuzzles = (uint32_t**)malloc(sizeof(uint32_t*)*nPuzzles);
  speedUps = (uint32_t*)malloc(sizeof(uint32_t)*nPuzzles);
  memset(speedUps,0,sizeof(uint32_t)*nPuzzles);

  for(i=0;i<nPuzzles;i++)
    {
      puzzles[i] = (uint32_t*)malloc(sizeof(uint32_t)*81);
      cpyPuzzles[i] = (uint32_t*)malloc(sizeof(uint32_t)*81);
    }

  j = i = 0;
  while((c  = fgetc(fp)) != EOF)
    {
      if(c == '\n')
        {
          j = 0;
          i++;
        }
      else if(c == ' ')
        {
          continue;
        }
      else if(c == '.')
        {
          puzzles[i][j++] = 0x1ff;
        }
      else
        {
          if(c >= 65 && c <= 70)
            {
              c -= 55;
            }
          else
            {
              c -= 48;
            }
          puzzles[i][j++] = 1 << (c-1);
        }
    }
  fclose(fp);
  for(i=0;i<nPuzzles;i++)
    {
      memcpy(cpyPuzzles[i], puzzles[i], sizeof(uint32_t)*81);
    }

  printf("nPuzzles=%d\n", nPuzzles);
  cpuBoard = (uint32_t*)malloc(sizeof(uint32_t)*81);
  hwBoard = (uint32_t*)malloc(sizeof(uint32_t)*81);
  os = (uint32_t*)malloc(sizeof(uint32_t)*81);
  uint32_t mismatches = 0;
  
  double t0,t1;
  for(i=0;i<nPuzzles;i++)
    {
      c0 = c1 = 1;
      t0 = t1 = 0.0;
      
      memcpy(cpuBoard,puzzles[i],sizeof(uint32_t)*81);
      memcpy(hwBoard,puzzles[i],sizeof(uint32_t)*81);

      c0 = getTicks(cCnt);
      i0 = getInsns(iCnt); 
      c = sudoku(cpuBoard, os);
      i0 = getInsns(iCnt) - i0;
      c0 = getTicks(cCnt) - c0;
      
      double ipc = ((double)i0) / ((double)c0);

      c1 = getTicks(cCnt);
      rc = hw_solve(d,hwBoard);
      c1 = getTicks(cCnt) - c1;

      speedUps[i] = c0/c1;
      printf("SpeedUp = %llu (%d) : SW ticks = %llu (IPC = %g), HW ticks = %llu\n", 
	     c0/c1, c, c0, ipc, c1);
      
      if(rc != 1) 
	{
	  printf("error with puzzle %d\n", i);
	  //print_board(puzzles[i]);
	}
      

      for(j=0;j<81;j++)
	{
	  if(cpuBoard[j]!=hwBoard[j])
	    {
	      mismatches++;
	      break;
	    }
	}
      
      //print_board(puzzles[i]);
    }

 done:
  printf("%u mismatches between CPU and FPGA\n", mismatches);
  qsort(speedUps, nPuzzles, sizeof(uint32_t), u32_cmp);
  c = rand()%nPuzzles;
  
  for(i=0;i<nPuzzles;i++)
    {
      memcpy(puzzles[i], cpyPuzzles[i], sizeof(uint32_t)*81);
    }

  t1 = timestamp();
  for(i=0;i<nPuzzles;i++)
    {
      sudoku(puzzles[i],os);
    }
  t1 = timestamp() - t1;
  printf("%d\n", puzzles[c][0]);
  
  for(i=0;i<nPuzzles;i++)
    {
      memcpy(puzzles[i], cpyPuzzles[i], sizeof(uint32_t)*81);
    }

  t0 = timestamp();
  for(i=0;i<nPuzzles;i++)
   {
     hw_solve(d,puzzles[i]);
   }
  t0 = timestamp() - t0;

  printf("%d\n", puzzles[c][0]);

  printf("HW=%g,SW=%g\n", t0, t1);
  if(nPuzzles == 1)
    {
      print_board(puzzles[0]);
    }

  if(nPuzzles > 0)
    {
      printf("Speed-Ups: Median = %u, Max=%u, Min=%u\n", 
	     speedUps[nPuzzles/2], 
	     speedUps[0], 
	     speedUps[nPuzzles-1]
	     );
    }
  for(i=0;i<nPuzzles;i++)
    {
      free(cpyPuzzles[i]);
      free(puzzles[i]);
    }
  free(cpyPuzzles);
  free(puzzles);

  delete d;
  free(os); 
  free(cpuBoard);
  free(hwBoard);
  free(speedUps);
  return 0;
}

int32_t solve(uint32_t *board, uint32_t *os)
{
  int32_t i,j,idx;
  int32_t ii,jj;
  int32_t ib,jb;
  uint32_t set_row, set_col, set_sqr;
  uint32_t row_or, col_or, sqr_or;
  uint32_t tmp;
  int32_t changed = 0;
    
  do
    {
      changed=0;
      //print_board(board);
      /* compute all positions one's set value */
      for(i = 0; i < 9; i++)
	{
	  for(j = 0; j < 9; j++)
	    {
	      idx = i*9 + j;
	      os[idx] = one_set(board[idx]);
	    }
	}

      for(i = 0; i < 9; i++)
	{
	  for(j = 0; j < 9; j++)
	    {
	      /* already solved */
	      if(isPow2(board[i*9+j]))
		continue;
	      else if(board[idx]==0)
		return 0;

	      row_or = set_row = 0;
	      for(jj = 0; jj < 9; jj++)
		{
		  idx = i*9 + jj;
		  if(jj == j)
		    continue;
		  set_row |= os[idx];
		  row_or |= board[idx];
		}
	      col_or = set_col = 0;
	      for(ii = 0; ii < 9; ii++)
		{
		  idx = ii*9 + j;
		  if(ii == i)
		    continue;
		  set_col |= os[idx];
		  col_or |= board[idx];
		}
	      sqr_or = set_sqr = 0;
#ifdef USE_DIV_TABLE
	      ib = divTable[i];
	      jb = divTable[j];
#else
	      ib = 3*(i/3);
	      jb = 3*(j/3);
#endif
	      for(ii=ib;ii < ib+3;ii++)
		{
		  for(jj=jb;jj<jb+3;jj++)
		    {
		      idx = ii*9 + jj;
		      if((i==ii) && (j == jj))
			continue;
		      set_sqr |= os[idx];
		      sqr_or |= board[idx];
		    }
		}
	      tmp = board[i*9 + j] & ~( set_row | set_col | set_sqr);
	      	      
	      if(tmp != board[i*9 + j])
		{
		  changed = 1;
		}
	      board[i*9+j] = tmp;

	      /* check for singletons */
	      tmp = 0;
	      tmp = one_set(board[i*9 + j] & (~row_or));
	      tmp |= one_set(board[i*9 + j] & (~col_or));
	      tmp |= one_set(board[i*9 + j] & (~sqr_or));
	      if(tmp != 0 && (board[i*9+j] != tmp))
		{
		  board[i*9+j] = tmp;
		  changed = 1;
		}
	    }
	}
      
    } while(changed);

  return 0;
}

inline void find_min(uint32_t *board, int32_t *min_idx, int *min_pos)
{
  int32_t tmp,idx,i,j;
  int32_t tmin_idx,tmin_pos;
  
  tmin_idx = 0;
  tmin_pos = INT_MAX;
  for(idx=0;idx<81;idx++)
    {
      tmp = count_ones(board[idx]);
      tmp = (tmp == 1) ? INT_MAX : tmp;
      if(tmp < tmin_pos)
	{
	  tmin_pos = tmp;
	  tmin_idx = idx;
	}
    }
  *min_idx = tmin_idx;
  *min_pos = tmin_pos;
}

int32_t sudoku(uint32_t *board, uint32_t *os)
{
  int32_t rc;
  int32_t itrs=0;
  int32_t tmp,min_pos;
  int32_t min_idx;
  int32_t i,j,idx;
    
  uint32_t cell;
  uint32_t old[81];
 
  uint32_t unsolved_pieces = 0;
  uint32_t *bptr, *nbptr;

  int32_t stack_pos = 0;
  int32_t stack_size = (1<<6);
  uint32_t **stack = 0;
  
  stack = (uint32_t**)malloc(sizeof(uint32_t*)*stack_size);
  for(i=0;i<stack_size;i++)
    {
      stack[i] = (uint32_t*)malloc(sizeof(uint32_t)*81);
    }

  memcpy(stack[stack_pos++], board, sizeof(uint32_t)*81);

  //printf("%d poss\n", count_poss(board));
  while(stack_pos > 0)
    {
      itrs++;
      unsolved_pieces = 0;
      bptr = stack[--stack_pos];
      
      bzero(os,sizeof(uint32_t)*81);
      solve(bptr,os);
      rc = check_correct(bptr, &unsolved_pieces);
      /* solved puzzle */
      if(rc == 1 && unsolved_pieces == 0)
	{
	  memcpy(board, bptr, sizeof(uint32_t)*81);
	  goto solved_puzzle;
	}
      /* traversed to bottom of search tree and
       * didn't find a valid solution */
      if(rc == 0 && unsolved_pieces == 0)
	{
	  continue;
	}
      
      find_min(bptr, &min_idx, &min_pos);
      cell = bptr[min_idx];  
      while(cell != 0)
	{
	  tmp = find_first_set(cell);
	  cell &= ~(1 << tmp);
	  nbptr = stack[stack_pos];
	  stack_pos++;
	  memcpy(nbptr, bptr, sizeof(uint32_t)*81);
	  nbptr[min_idx] = 1<<tmp;
	  
	  assert(stack_pos < stack_size);
	}
    }
 solved_puzzle:
    
  for(i=0;i<stack_size;i++)
    {
      free(stack[i]);
    } 
  free(stack);

  return itrs;
}
