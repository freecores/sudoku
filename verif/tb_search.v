module tb_search();
   reg clk;
   reg rst;
   reg start;
   wire done;
  
   wire [728:0] outGrid;
   wire [728:0] inGrid;

   reg [8:0] 	mem [80:0];

        
   initial
     begin
	clk = 0;
	rst = 1;

	$readmemh("puzzle_9.hex", mem);
	$display("mem[0] = %x, mem[80] = %x",
		 mem[0], mem[80]);	
      	#1000
	  rst = 0;
     end

   always@(posedge clk)
     begin
	if(rst)
	  start <= 1'b1;
	else
	  start <= start ? 1'b0 : start;
     end
   
   always
     clk = #5 !clk;

  
         
   sudoku_search uut (
	       // Outputs
	       .outGrid			(outGrid[728:0]),
	       .done                    (done),
	       .error                   (),
	       // Inputs
	       .clk			(clk),
	       .rst			(rst),
	       .start			(start),
	       .inGrid			(inGrid[728:0])
	       );

   assign inGrid = {mem[0],mem[1],mem[2],mem[3],mem[4],mem[5],mem[6],mem[7],mem[8],mem[9],mem[10],mem[11],mem[12],mem[13],mem[14],mem[15],mem[16],mem[17],mem[18],mem[19],mem[20],mem[21],mem[22],mem[23],mem[24],mem[25],mem[26],mem[27],mem[28],mem[29],mem[30],mem[31],mem[32],mem[33],mem[34],mem[35],mem[36],mem[37],mem[38],mem[39],mem[40],mem[41],mem[42],mem[43],mem[44],mem[45],mem[46],mem[47],mem[48],mem[49],mem[50],mem[51],mem[52],mem[53],mem[54],mem[55],mem[56],mem[57],mem[58],mem[59],mem[60],mem[61],mem[62],mem[63],mem[64],mem[65],mem[66],mem[67],mem[68],mem[69],mem[70],mem[71],mem[72],mem[73],mem[74],mem[75],mem[76],mem[77],mem[78],mem[79],mem[80]};

   reg [31:0] r_cnt;
  

   wire [8:0] result [80:0];
   wire [8:0] result_dec [80:0];
   
   genvar     i;
   integer    y,x;
     
   generate
      for(i=0;i<81;i=i+1)
	begin: unflatten
	   assign result[i] = outGrid[(9*(i+1))-1:9*i];
	   hot2dec h (.hot(result[i]), .dec(result_dec[i]));
	end
   endgenerate

  
   always@(posedge clk)
     begin
	if(rst)
	  begin
	     r_cnt <= 32'd0;
	  end
	else
	  begin
	     r_cnt <= start ? 32'd0 : r_cnt + 32'd1;
	     if(done)
	       begin
		  $write("\n");
		  for(y=0;y<9;y=y+1)
		    begin
		       for(x=0;x<9;x=x+1)
			 begin
			    $write("%d ", result_dec[y*9+x]);
			 end
		       $write("\n");
		    end
		  $display("solved in %d cycles", r_cnt);
		  $finish();
	       end // if (done)
	  end // else: !if(rst)
     end // always@ (posedge clk)
 
      
endmodule // tb_search


module hot2dec(input [8:0] hot, output [8:0] dec);
   assign dec = (hot == 9'd1) ? 9'd1 :
		(hot == 9'd2) ? 9'd2 :
		(hot == 9'd4) ? 9'd3 :
		(hot == 9'd8) ? 9'd4 :
		(hot == 9'd16) ? 9'd5 :
		(hot == 9'd32) ? 9'd6 :
		(hot == 9'd64) ? 9'd7 :
		(hot == 9'd128) ? 9'd8 :
		(hot == 9'd256) ? 9'd9 :
		9'd0;
endmodule