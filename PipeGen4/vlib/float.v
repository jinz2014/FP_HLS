module fadd (
    input clk,
    input [31:0] a,
    input [31:0] b,
    input go ,
    input pipeEn ,
    output [31:0] result,
    output rdy 
  );

  xilinx_fadd xilinx_fadd_i (
      .a ( a ),
      .b ( b ),
      .operation_nd ( go ),
      .clk ( clk ),
      .result ( result ),
      .ce ( pipeEn ),
      .rdy ( rdy )
    );

endmodule

module fsub (
    input clk,
    input [31:0] a,
    input [31:0] b,
    input go ,
    input pipeEn ,
    output [31:0] result,
    output rdy 
  );

  xilinx_fsub xilinx_fsub_i (
      .a ( a ),
      .b ( b ),
      .operation_nd ( go ),
      .clk ( clk ),
      .result ( result ),
      .ce ( pipeEn ),
      .rdy ( rdy )
    );

endmodule

module fmul (
    input clk,
    input [31:0] a,
    input [31:0] b,
    input go ,
    input pipeEn ,
    output [31:0] result,
    output rdy 
  );

  xilinx_fmul xilinx_fmul_i (
      .a ( a ),
      .b ( b ),
      .operation_nd ( go ),
      .clk ( clk ),
      .result ( result ),
      .ce ( pipeEn ),
      .rdy ( rdy )
    );

endmodule

module fdiv (
    input clk,
    input [31:0] a,
    input [31:0] b,
    input go ,
    input pipeEn ,
    output reg [31:0] result,
    output reg done 
  );

  wire [31:0] result_tmp;
  wire        rdy_tmp;

  xilinx_fdiv xilinx_fdiv_i (
      .a ( a ),
      .b ( b ),
      .operation_nd ( go ),
      .clk ( clk ),
      .result ( result_tmp ),
      .ce ( pipeEn ),
      .rdy ( rdy_tmp )
    );

  always @ (posedge clk) begin
    done   <= rdy_tmp;
    if (rdy_tmp && pipeEn) 
      result <= result_tmp;
  end

endmodule
