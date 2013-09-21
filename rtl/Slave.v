`include "piece.v"
`include "sudoku.v"
`include "sudoku_search.v"
`include "wrap_search.v"
`include "minPiece.v"


module Slave(input clk, 
	     input 	   reset,
	     input [7:0]   io_write_valid,
	     output 	   io_write_ready,
	     input [3:0]   io_write_byteEnable,
	     input [31:0]  io_write_data,
	     input [7:0]   io_read_valid,
	     output 	   io_read_ready,
	     output [31:0] io_read_data);

   /* DBS verilog notes:
    * rst -> active high reset...
    * io_write_valid -> one hot code for write
    * io_read_valid -> one hot code for read
    */
   reg [31:0] 		   rf[7:0];

   wire w_read = |io_read_valid;
   wire w_write = |io_write_valid;
   
   assign io_read_ready = |io_read_valid;
   assign io_write_ready = |io_write_valid;
   
   wire [2:0] 		   r_addr;
   wire [2:0] 		   w_addr;
		   
   assign r_addr = io_read_valid == 8'd1 ? 3'd0 :
		   io_read_valid == 8'd2 ? 3'd1 :
		   io_read_valid == 8'd4 ? 3'd2 :
		   io_read_valid == 8'd8 ? 3'd3 :
		   io_read_valid == 8'd16 ? 3'd4 :
		   io_read_valid == 8'd32 ? 3'd5 :
		   io_read_valid == 8'd64 ? 3'd6 : 
		   3'd7;

   assign w_addr = io_write_valid == 8'd1 ? 3'd0 :
		   io_write_valid == 8'd2 ? 3'd1 :
		   io_write_valid == 8'd4 ? 3'd2 :
		   io_write_valid == 8'd8 ? 3'd3 :
		   io_write_valid == 8'd16 ? 3'd4 :
		   io_write_valid == 8'd32 ? 3'd5 :
		   io_write_valid == 8'd64 ? 3'd6 : 
		   3'd7;
   
   assign io_read_data =  w_read ? rf[r_addr] : 32'hffffffff;

   wire [8:0] 		   w_dout;
     wire 		   w_start;

   assign w_start = rf[7][0];

   wire 		   w_wr;
   wire [6:0] 		   w_saddr;
   wire [8:0] 		   w_din;
   assign w_wr = rf[1][31];
   assign w_saddr = rf[1][6:0];
   assign w_din = rf[1][15:7];
   
  
   wire 		   w_done, w_error;
   
   always@(posedge clk)
     begin
	if(reset)
	  begin
	     rf[0] <= 32'd0;
	     rf[1] <= 32'd0;
	     rf[2] <= 32'd0;
	     rf[3] <= 32'd0;
	     rf[7] <= 32'd0;
	  end
	else
	  begin
	     rf[0] <= {32'd0, w_error, w_done};
	     /* fire for one cycle */
	     rf[7] <= (w_write && (w_addr == 3'd7)) ? io_write_data : 32'd0;

	     if(w_write && (w_addr == 3'd1))
	       begin
		  rf[1] <= io_write_data;
	       end
	     rf[2] <= {23'd0, w_dout};
	     	     
	  end
     end
   
    wrap_search solver0 (
			// Outputs
			.dout		(w_dout),
			.done           (w_done),
			.error          (w_error),
			// Inputs
			.clk		(clk),
			.rst		(reset),
			.start		(w_start),
			.addr		(w_saddr),
			.wr		(w_wr),
			.din		(w_din)
			);

 

	
endmodule

