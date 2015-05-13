//----------------------------------------------------------------------------
// For simplicity of generation, here are registered ouput signals of mrbays
// if fadd latency = 11, it is fadd1
// if fadd latency = 12, it is fadd2
//----------------------------------------------------------------------------
fprintf(vfp, "reg [15:0] clP0_cnt, clP1_cnt, clP2_cnt, clP3_cnt;\n");
fprintf(vfp, "reg [15:0] scP_cnt, lnScaler_cnt, lnL_cnt;\n\n");
fprintf(vfp, "always @ (posedge clk) begin   \n");
fprintf(vfp, "  if (rst) begin               \n");
fprintf(vfp, "    clP0_cnt <= 0;             \n");
fprintf(vfp, "    clP1_cnt <= 0;             \n");
fprintf(vfp, "    clP2_cnt <= 0;             \n");
fprintf(vfp, "    clP3_cnt <= 0;             \n");
fprintf(vfp, "    scP_cnt  <= 0;             \n");
fprintf(vfp, "    lnScaler_cnt <= 0;         \n");
fprintf(vfp, "    lnL_cnt  <= 0;             \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && mrbay_fmux5_res_rdy) begin  \n");
fprintf(vfp, "    clP0_cnt <= clP0_cnt + 1;                \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fmux6_res_rdy) begin  \n");
fprintf(vfp, "    clP1_cnt <= clP1_cnt + 1;                \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fmux11_res_rdy) begin  \n");
fprintf(vfp, "    clP2_cnt <= clP2_cnt + 1;                \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fmux12_res_rdy) begin  \n");
fprintf(vfp, "    clP3_cnt <= clP3_cnt + 1;                \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fadd5_res_rdy) begin   \n");
fprintf(vfp, "    scP_cnt <= scP_cnt + 1;                  \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fadd6_res_rdy) begin   \n");
fprintf(vfp, "    lnScaler_cnt <= lnScaler_cnt + 1;        \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "  if (!stall && mrbay_fmul14_res_rdy) begin  \n");
fprintf(vfp, "    lnL_cnt <= lnL_cnt + 1;                  \n");
fprintf(vfp, "  end                                        \n");
fprintf(vfp, "end                                          \n\n");

fprintf(vfp, "always @ (posedge clk) begin   \n");
fprintf(vfp, "  mrbay_fmux5_out_rdy <= ~rst & mrbay_fmux5_res_rdy;\n");
fprintf(vfp, "  mrbay_fmux6_out_rdy <= ~rst & mrbay_fmux6_res_rdy;\n");
fprintf(vfp, "  mrbay_fmux11_out_rdy <= ~rst & mrbay_fmux11_res_rdy;\n");
fprintf(vfp, "  mrbay_fmux12_out_rdy <= ~rst & mrbay_fmux12_res_rdy;\n");
fprintf(vfp, "  mrbay_fadd5_out_rdy <= ~rst & mrbay_fadd5_res_rdy;\n");
fprintf(vfp, "  mrbay_fadd6_out_rdy  <= ~rst & mrbay_fadd6_res_rdy;\n");
fprintf(vfp, "  mrbay_fmul14_out_rdy <= ~rst & mrbay_fmul14_res_rdy;\n");
fprintf(vfp, "end                            \n\n");

fprintf(vfp, "always @ (posedge clk) begin   \n");
fprintf(vfp, "  mrbay_fmux5_out <= mrbay_fmux5_res;\n");
fprintf(vfp, "  mrbay_fmux6_out <= mrbay_fmux6_res;\n");
fprintf(vfp, "  mrbay_fmux11_out <= mrbay_fmux11_res;\n");
fprintf(vfp, "  mrbay_fmux12_out <= mrbay_fmux12_res;\n");
fprintf(vfp, "  mrbay_fadd5_out <= mrbay_fadd5_res;\n");
fprintf(vfp, "  mrbay_fadd6_out  <= mrbay_fadd6_res;\n");
fprintf(vfp, "  mrbay_fmul14_out <= mrbay_fmul14_res;\n");
fprintf(vfp, "end                            \n\n");

