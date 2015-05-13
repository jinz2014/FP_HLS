//
module MUX2 (
  S, 
  I1, 
  I2, 
  Z);

 parameter DataWidth = 32;

 input S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 output [DataWidth-1:0] Z;

 assign Z = S ? I2 : I1;

endmodule 

/*
When Low, S selects DI; when High, S selects CI

Verilog Instantiation Template
MUXCY MUXCY_instance_name (.O (user_O),
                           .CI (user_CI),
                           .DI (user_DI),
                           .S (user_S)); 
module MUX2 (
  S, 
  I1, 
  I2, 
  Z);

 parameter DataWidth = 32;

 input S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 output [DataWidth-1:0] Z;

 //assign Z = S ? I2 : I1;
 genvar i;


generate for (i = 0; i < DataWidth; i = i + 1) begin : MUX2_32
 MUXCY MUXCY(.O(Z[i]), .CI(I2[i]), .DI(I1[i]), .S(S));
end endgenerate

endmodule 
*/

   
module MUX3 (
  S, 
  I1, 
  I2, 
  I3, 
  Z);

 parameter DataWidth = 32;

 input [1:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, S)
   (* parallel_case *) case (S)
     2'd0: Z = I1;
     2'd1: Z = I2;
     2'd2: Z = I3;
     //default: $display("Error in S");
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 


module MUX4 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  Z);

 parameter DataWidth = 32;

 input [1:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 output reg [DataWidth-1:0] Z;

/* slice 19  slice luts 32 */
 always @ (I1, I2, I3, I4, S)
   case (S)
     2'd0: Z = I1;
     2'd1: Z = I2;
     2'd2: Z = I3;
     2'd3: Z = I4;
   endcase

endmodule 

module MUX4_hot (
  a,
  b,
  c, 
  I1, 
  I2, 
  I3, 
  I4, 
  Z);

 parameter DataWidth = 32;

 input a,b,c;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 output [DataWidth-1:0] Z;

 /* slice 20  slice luts 33
 assign t1 = a ? I1 : I2 ;
 assign t2 = b ? t1 : I3;
 assign Z  = c ? t2 : I4; */

 assign t1 = a ? I1 : I2 ;
 assign t2 = b ? I3 : t1;
 assign Z  = c ? I4 : t2;


endmodule 

module MUX5 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  Z);

 parameter DataWidth = 32;

 input [2:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, S)
   //(* parallel_case *) case (S)
   case (S)
     3'd0: Z = I1;
     3'd1: Z = I2;
     3'd2: Z = I3;
     3'd3: Z = I4;
     3'd4: Z = I5;
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 

module MUX6 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  Z);

 parameter DataWidth = 32;

 input [2:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, S)
   //(* parallel_case *) case (S)
   case (S)
     3'd0: Z = I1;
     3'd1: Z = I2;
     3'd2: Z = I3;
     3'd3: Z = I4;
     3'd4: Z = I5;
     3'd5: Z = I6;
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 

module MUX7 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  Z);

 parameter DataWidth = 32;

 input [2:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, I7, S)
   //(* parallel_case *) case (S)
   case (S)
     3'd0: Z = I1;
     3'd1: Z = I2;
     3'd2: Z = I3;
     3'd3: Z = I4;
     3'd4: Z = I5;
     3'd5: Z = I6;
     3'd6: Z = I7;
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 

module MUX8 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  I8, 
  Z);

 parameter DataWidth = 32;

 input [2:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 input [DataWidth-1:0] I8;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, I7, I8, S)
   case (S)
     3'd0: Z = I1;
     3'd1: Z = I2;
     3'd2: Z = I3;
     3'd3: Z = I4;
     3'd4: Z = I5;
     3'd5: Z = I6;
     3'd6: Z = I7;
     3'd7: Z = I8;
   endcase

endmodule
 
module MUX8_onehot (
  s1, s2, s3, s4, s5, s6, s7,
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  I8, 
  Z);

 parameter DataWidth = 32;

 input s1, s2, s3, s4, s5, s6, s7;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 input [DataWidth-1:0] I8;
 output[DataWidth-1:0] Z;

  wire [31:0] o1, o2, o3, o4, o5, o6;

   //assign Z =  s7 ? I8 : s6 ? I7 : s5 ? I6 : s4 ? I5 : s3 ? I4 : s2 ? I3 : s1 ? I2 : I1;
  MUX2 m1 (s1, I1, I2, o1);
  MUX2 m2 (s2, o1, I3, o2);
  MUX2 m3 (s3, o2, I4, o3);
  MUX2 m4 (s4, o3, I5, o4);
  MUX2 m5 (s5, o4, I6, o5);
  MUX2 m6 (s6, o5, I7, o6);
  MUX2 m7 (s7, o6, I8, Z);
  
endmodule 


module MUX9 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  I8, 
  I9, 
  Z);

 parameter DataWidth = 32;

 input [3:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 input [DataWidth-1:0] I8;
 input [DataWidth-1:0] I9;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, I7, I8, I9, S)
   //(* parallel_case *) case (S)
   case (S)
     4'd0: Z = I1;
     4'd1: Z = I2;
     4'd2: Z = I3;
     4'd3: Z = I4;
     4'd4: Z = I5;
     4'd5: Z = I6;
     4'd6: Z = I7;
     4'd7: Z = I8;
     4'd8: Z = I9;
     default: Z = {DataWidth{1'bx}};
   endcase
endmodule 


module MUX10 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  I8, 
  I9, 
  I10, 
  Z);

 parameter DataWidth = 32;

 input [3:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 input [DataWidth-1:0] I8;
 input [DataWidth-1:0] I9;
 input [DataWidth-1:0] I10;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, I7, I8, I9, I10, S)
   //(* parallel_case *) case (S)
   case (S)
     4'd0: Z = I1;
     4'd1: Z = I2;
     4'd2: Z = I3;
     4'd3: Z = I4;
     4'd4: Z = I5;
     4'd5: Z = I6;
     4'd6: Z = I7;
     4'd7: Z = I8;
     4'd8: Z = I9;
     4'd9: Z = I10;
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 

module MUX11 (
  S, 
  I1, 
  I2, 
  I3, 
  I4, 
  I5, 
  I6, 
  I7, 
  I8, 
  I9, 
  I10, 
  I11, 
  Z);

 parameter DataWidth = 32;

 input [3:0] S;
 input [DataWidth-1:0] I1;
 input [DataWidth-1:0] I2;
 input [DataWidth-1:0] I3;
 input [DataWidth-1:0] I4;
 input [DataWidth-1:0] I5;
 input [DataWidth-1:0] I6;
 input [DataWidth-1:0] I7;
 input [DataWidth-1:0] I8;
 input [DataWidth-1:0] I9;
 input [DataWidth-1:0] I10;
 input [DataWidth-1:0] I11;
 output reg [DataWidth-1:0] Z;

 always @ (I1, I2, I3, I4, I5, I6, I7, I8, I9, I10, I11, S)
   //(* parallel_case *) case (S)
   case (S)
     4'd0: Z = I1;
     4'd1: Z = I2;
     4'd2: Z = I3;
     4'd3: Z = I4;
     4'd4: Z = I5;
     4'd5: Z = I6;
     4'd6: Z = I7;
     4'd7: Z = I8;
     4'd8: Z = I9;
     4'd9: Z = I10;
     4'd10: Z = I11;
     default: Z = {DataWidth{1'bx}};
   endcase

endmodule 
