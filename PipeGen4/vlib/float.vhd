Library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity faddsub is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    op: in std_ulogic_vector(5 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(31 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end faddsub;

architecture faddsub_a of faddsub is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal operation: std_logic_vector(5 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_faddsub : floating_point_v4_0
    generic map(
      c_has_add => 1,                 -- 000000
      c_has_subtract => 1,            -- 000001
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 11,
      --c_latency => 5,
      --c_latency => 4,
      c_has_rdy => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation => operation,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  operation <= std_logic_vector(op);
  result <= std_ulogic_vector(result_slv);
  
end faddsub_a;

Library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fadd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(31 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end fadd;

architecture fadd_a of fadd is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_fadd : floating_point_v4_0
    generic map(
      c_has_add => 1,
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 11, --3,
      c_has_rdy => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end fadd_a;

library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fsub is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(31 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end fsub;

architecture fsub_a of fsub is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_fsub : floating_point_v4_0
    generic map(
      c_has_subtract => 1,
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 11,
      c_has_rdy => 1
    )
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end fsub_a;

library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fmul is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(31 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end fmul;

architecture fmul_a of fmul is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd: std_logic;
begin
  operation_nd <= go;

  xilinx_fmul : floating_point_v4_0
    generic map(
      c_has_multiply => 1,
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,    
      c_latency => 6, --3,
      c_has_rdy => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end fmul_a;

-----------------------------------------------------------------------------
-- floating-point divider
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fdiv is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(31 downto 0);
    pipeEn: in std_ulogic;
    done: out std_ulogic);
end fdiv;

architecture fdiv_a of fdiv is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd, rdy, rfd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_fdiv : floating_point_v4_0
    generic map(
      c_b_fraction_width => 24,
      c_has_operation_nd => 1,
      c_a_fraction_width => 24,
      -- div result is registered (see below)
      c_latency => 28, --26,
      c_has_ce=> 1,
      c_has_rdy => 1,
      c_result_fraction_width => 24,
      c_has_divide => 1,
      --c_rate => 26,
      c_rate => 1,
      c_has_operation_rfd => 1,
      c_result_width => 32,
      c_b_width => 32,
      c_a_width => 32)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      operation_rfd => rfd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  process (clk)
  begin  -- process
    if clk'event and clk = '1' then  -- rising clock edge
      if (rdy = '1' and pipeEn = '1') then
        result <= std_ulogic_vector(result_slv); 
      end if;
    end if;
  end process;
  
  --ack0: entity work.impulse_acknowledge port map (clk, go, rdy, done);
  
end fdiv_a;

-----------------------------------------------------------------------------
-- floating-point less-than operator
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_consts.all;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity flt is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(0 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end flt;

architecture flt_a of flt is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_flt : floating_point_v4_0
    generic map(
      c_has_compare => 1,
      c_compare_operation => FLT_PT_LESS_THAN,
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      rdy => rdy,
      ce => pipeEn);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result(0) <= result_slv(0);
  
end flt_a;

-----------------------------------------------------------------------------
-- floating-point greater-than operator
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_consts.all;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fgt is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    b: in std_ulogic_vector(31 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(0 downto 0);
    pipeEn: in std_ulogic;
    rdy: out std_ulogic);
end fgt;

architecture fgt_a of fgt is
    signal a_slv: std_logic_vector(31 downto 0);
    signal b_slv: std_logic_vector(31 downto 0);
    signal result_slv: std_logic_vector(31 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_fgt : floating_point_v4_0
    generic map(
      c_has_compare => 1,
      c_compare_operation => FLT_PT_GREATER_THAN,
      c_a_width => 32,
      c_a_fraction_width => 24,
      c_b_width => 32,
      c_b_fraction_width => 24,
      c_result_fraction_width => 24,
      c_result_width => 32,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      rdy => rdy,
      ce => pipeEn);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result(0) <= result_slv(0);
  
end fgt_a;


-----------------------------------------------------------------------------
-- single -> double conversion
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity ftod is
  port (
    --clk: in std_ulogic;
    a: in std_ulogic_vector(31 downto 0);
    result: out std_ulogic_vector(63 downto 0));
end ftod;

architecture ftod_a of ftod is
    signal exp_s : std_ulogic_vector(7 downto 0);
    signal exp_d : unsigned(10 downto 0);
    signal zero : std_ulogic;
begin
  exp_s <= a(30 downto 23);             -- extract the exponent
  exp_d <= unsigned("000" & exp_s) + 896;  -- rebias
  zero <= '1' when a = X"00000000" else '0';

  result(63) <= a(31);                  -- sign bit
   -- biased exponent
  result(62 downto 52) <= std_ulogic_vector(exp_d) when zero = '0'
                          else "00000000000";
  result(51 downto 29) <= a(22 downto 0);  -- fraction
  result(28 downto 0) <= "00000000000000000000000000000";
end ftod_a;
  
-----------------------------------------------------------------------------
-- double -> single conversion
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity dtof is
  port (
    --clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    result: out std_ulogic_vector(31 downto 0));
end dtof;

architecture dtof_a of dtof is
    signal exp_s : std_ulogic_vector(7 downto 0);
    signal exp_d : std_ulogic_vector(10 downto 0);
    signal exp : unsigned(10 downto 0);
    signal ovr, inf : std_ulogic;
begin
  exp_d <= a(62 downto 52);             -- extract the exponent
  exp <= unsigned(exp_d) - 896;  -- rebias
  exp_s <= std_ulogic_vector(exp(7 downto 0));
  ovr <= '0' when std_ulogic_vector(exp(10 downto 8)) = "000" else '1';
  inf <= a(62) when ovr = '1' else '0';

  -- sign bit
  result(31) <= a(63);
  -- exponent
  result(30 downto 23) <= exp_s when ovr = '0'
                          else "00000000" when inf = '0'
                          else "11111111";
  -- fraction
  result(22 downto 0) <= a(51 downto 29) when ovr = '0'
                         else "00000000000000000000000";
end dtof_a;


library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_consts.all;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fltd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(0 downto 0);
    rdy: out std_ulogic;
    pipeEn: in std_ulogic);
end fltd;

architecture fltd_a of fltd is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_flt : floating_point_v4_0
    generic map(
      c_has_compare => 1,
      c_compare_operation => FLT_PT_LESS_THAN,
      c_a_width => 64,
      c_a_fraction_width => 53,
      c_b_width => 64,
      c_b_fraction_width => 53,
      c_result_fraction_width => 53,
      c_result_width => 64,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result(0) <= result_slv(0);
  
end fltd_a;


library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_consts.all;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fgtd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(0 downto 0);
    rdy: out std_ulogic;
    pipeEn: in std_ulogic);
end fgtd;

architecture fgtd_a of fgtd is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd: std_logic;
begin
  operation_nd <= go;
  
  xilinx_fgt : floating_point_v4_0
    generic map(
      c_has_compare => 1,
      c_compare_operation => FLT_PT_GREATER_THAN,
      c_a_width => 64,
      c_a_fraction_width => 53,
      c_b_width => 64,
      c_b_fraction_width => 53,
      c_result_fraction_width => 53,
      c_result_width => 64,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 1)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result(0) <= result_slv(0);
  
end fgtd_a;


library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity faddd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(63 downto 0);
    rdy: out std_ulogic;
    pipeEn: in std_ulogic);
end faddd;

architecture faddd_a of faddd is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd: std_logic;
begin
  operation_nd <= go;
  
  xilinx_fadd : floating_point_v4_0
    generic map(
      c_has_add => 1,
      c_a_width => 64,
      c_a_fraction_width => 53,
      c_b_width => 64,
      c_b_fraction_width => 53,
      c_result_fraction_width => 53,
      c_result_width => 64,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 12) -- xilinx luts
      --c_latency => 14) -- xilinx dsp48e
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end faddd_a;

library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fsubd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(63 downto 0);
    rdy: out std_ulogic;
    pipeEn: in std_ulogic);
end fsubd;

architecture fsubd_a of fsubd is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd: std_logic;
begin
  operation_nd <= go;
  
  xilinx_fsub : floating_point_v4_0
    generic map(
      c_has_subtract => 1,
      c_a_width => 64,
      c_a_fraction_width => 53,
      c_b_width => 64,
      c_b_fraction_width => 53,
      c_result_fraction_width => 53,
      c_result_width => 64,
      c_has_ce=> 1,
      c_has_rdy=> 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 14)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end fsubd_a;

library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fmuld is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(63 downto 0);
    rdy: out std_ulogic;
    pipeEn: in std_ulogic);
end fmuld;

architecture fmuld_a of fmuld is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd: std_logic;
begin
  operation_nd <= go;

  xilinx_fmul : floating_point_v4_0
    generic map(
      c_has_multiply => 1,
      c_a_width => 64,
      c_a_fraction_width => 53,
      c_b_width => 64,
      c_b_fraction_width => 53,
      c_result_fraction_width => 53,
      c_result_width => 64,
      c_has_ce=> 1,
      c_has_rdy => 1,
      c_has_operation_nd => 1,
      c_rate => 1,
      c_latency => 15)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  result <= std_ulogic_vector(result_slv);
  
end fmuld_a;

library ieee;
use ieee.std_logic_1164.all;
library XilinxCoreLib;
use XilinxCoreLib.floating_point_v4_0_comp.all;

entity fdivd is
  port (
    clk: in std_ulogic;
    a: in std_ulogic_vector(63 downto 0);
    b: in std_ulogic_vector(63 downto 0);
    go: in std_ulogic;
    result: out std_ulogic_vector(63 downto 0);
    pipeEn: in std_ulogic;
    done: out std_ulogic);
end fdivd;

architecture fdivd_a of fdivd is
    signal a_slv: std_logic_vector(63 downto 0);
    signal b_slv: std_logic_vector(63 downto 0);
    signal result_slv: std_logic_vector(63 downto 0);
    signal operation_nd, rdy, rfd : std_logic;
begin
  operation_nd <= go;
  
  xilinx_fdiv : floating_point_v4_0
    generic map(
      c_b_fraction_width => 53,
      c_has_operation_nd => 1,
      c_a_fraction_width => 53,
      c_latency => 56,
      c_has_ce=> 1,
      c_has_rdy => 1,
      c_result_fraction_width => 53,
      c_has_divide => 1,
      c_rate => 56,
      c_has_operation_rfd => 1,
      c_result_width => 64,
      c_b_width => 64,
      c_a_width => 64)
    port map (
      a => a_slv,
      b => b_slv,
      operation_nd => operation_nd,
      operation_rfd => rfd,
      clk => clk,
      result => result_slv,
      ce => pipeEn,
      rdy => rdy);

  a_slv <= std_logic_vector(a);
  b_slv <= std_logic_vector(b);
  process (clk)
  begin
    if clk'event and clk = '1' then
      if (rdy = '1' and pipeEn = '1') then
        result <= std_ulogic_vector(result_slv);
      end if;
    end if;
  end process;
  
end fdivd_a;
