module wrap_search(/*AUTOARG*/
   // Outputs
   dout, done, error,
   // Inputs
   clk, rst, start, addr, wr, din
   );
   input clk;
   input rst;
   input start;
   input [6:0] addr;
   input       wr;
   input [8:0] din;
   output [8:0] dout;

   output 	done;
   output 	error;
   

   wire [728:0]  outGrid;
   wire [728:0]  inGrid;
   
   wire [8:0] 	 outGrid2d [80:0];
   
   reg [8:0] 	 ram [127:0];

   assign dout = ram[addr];

   genvar 	 ii;
   generate
      for(ii=0;ii<81;ii=ii+1)
	begin: unflatten
	   assign outGrid2d[ii] = outGrid[(9*(ii+1))-1:9*ii];
	   assign inGrid[(9*(ii+1)-1):(9*ii)] = ram[ii];
	end
   endgenerate   


   integer 	 i;
   always@(posedge clk)
     begin
	if(wr)
	  begin
	     ram[addr] <= din;
	  end
	else if(done|error)
       	  begin
	     for(i = 0; i < 81; i=i+1)
	       begin
		  ram[i] <= outGrid2d[i];
	       end
	  end
     end
   

   sudoku_search solver0 
     (		   // Outputs
		   .outGrid		(outGrid[728:0]),
		   .done		(done),
		   .error            (error),
		   // Inuts
		   .clk			(clk),
		   .rst			(rst),
		   .start		(start),
		   .inGrid		(inGrid[728:0])
		   );

   
endmodule // wrap_sudoku