module RegWriteCtl (clk, rst, stall, start, wen);
  parameter DataWidth = 32;
  parameter DII = 5; 
  input clk;
  input rst;
  input stall;
  input start;
  output wen;

  reg [DataWidth-1:0] cnt;
  reg cnt_en;
  reg wen;

  always @ (posedge clk) begin
    if (rst) begin
      cnt_en <= 0;
      cnt <= 0;
      wen <= 0;
    end
    // assume state position > minII 
    else if (!stall) begin
      // this state[43] is not necessarily the control signal for
      // the last input data set. So state[43] just enables the 
      // counter and produce fen at the same as wen.
      if (start) begin 
        cnt_en <= 1;
        cnt <= cnt + 1'b1;
        wen <= 0;
      end
      else if (cnt_en) begin
        if (cnt == DII-1) begin // minII
          cnt <= 0;
          wen <= 1;
        end
        else begin
          cnt <= cnt + 1'b1;
          wen <= 0;
        end
      end
    end
  end
endmodule


/*
reg_ctl #(3, 5) r3_ctl (clk, rst, stall, state[3], r3_fen);
reg_ctl #(3, 5) r6_ctl (clk, rst, stall, state[34], r6_fen);
reg_ctl #(3, 5) r7_ctl (clk, rst, stall, state[43], r7_fen);
*/
