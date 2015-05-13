module ROM (clk, a0, a1, a2, a3, data);

  parameter DataWidth = 32;
  parameter MemDepth = 16;

  input  clk;
  input  a0, a1, a2, a3;
  output [DataWidth - 1:0] data;

  reg [DataWidth - 1:0] data;
  reg [DataWidth - 1:0] mem [0 : MemDepth-1];

  always @ (posedge clk) begin
    data <= mem[{a3, a2, a1, a0}];
  end

endmodule
