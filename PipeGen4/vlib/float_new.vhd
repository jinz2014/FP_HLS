--------------------------------------------------------
-- 
-- 
-- Instantiations of version 5 coregen components 
-- fadd, faddd, fmul, fmuld, fgt, fgtd, flt, fdiv 
--
-- Notes
-- XilinxCoreLib is not needed
--
-- /share/jin/SMBL/floatLib/
-- 
--------------------------------------------------------
Library ieee;
use ieee.std_logic_1164.all;

entity fadd is
  port (
    clk: in std_logic;
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    go : in STD_LOGIC;
    pipeEn : in STD_LOGIC;
    result: out std_logic_vector(31 downto 0);
    rdy : out STD_LOGIC);
end fadd;

architecture fadd_a of fadd is

  component xilinx_fadd
    port (
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    operation_nd: in std_logic;
    clk: in std_logic;
    ce: in std_logic;
    result: out std_logic_vector(31 downto 0);
    rdy: out std_logic);
  end component;

begin
 
  xilinx_fadd_i : xilinx_fadd
    port map (
      a => a,
      b => b,
      operation_nd => go,
      clk => clk,
      result => result,
      ce => pipeEn,
      rdy => rdy);
end fadd_a;

Library ieee;
use ieee.std_logic_1164.all;

entity fsub is
  port (
    clk: in std_logic;
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    go : in STD_LOGIC;
    pipeEn : in STD_LOGIC;
    result: out std_logic_vector(31 downto 0);
    rdy : out STD_LOGIC);
end fsub;

architecture fsub_a of fsub is

  component xilinx_fsub
    port (
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    operation_nd: in std_logic;
    clk: in std_logic;
    ce: in std_logic;
    result: out std_logic_vector(31 downto 0);
    rdy: out std_logic);
  end component;

begin
 
  xilinx_fsub_i : xilinx_fsub
    port map (
      a => a,
      b => b,
      operation_nd => go,
      clk => clk,
      result => result,
      ce => pipeEn,
      rdy => rdy);
end fsub_a;

Library ieee;
use ieee.std_logic_1164.all;

entity fmul is
  port (
    clk: in std_logic;
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    go : in STD_LOGIC;
    pipeEn : in STD_LOGIC;
    result: out std_logic_vector(31 downto 0);
    rdy : out STD_LOGIC);
end fmul;

architecture fmul_a of fmul is

  component xilinx_fmul
    port (
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    operation_nd: in std_logic;
    clk: in std_logic;
    ce: in std_logic;
    result: out std_logic_vector(31 downto 0);
    rdy: out std_logic);
  end component;

begin
 
  xilinx_fmul_i : xilinx_fmul
    port map (
      a => a,
      b => b,
      operation_nd => go,
      clk => clk,
      result => result,
      ce => pipeEn,
      rdy => rdy);
end fmul_a;

Library ieee;
use ieee.std_logic_1164.all;

entity fdiv is
  port (
    clk: in std_logic;
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    go : in STD_LOGIC;
    pipeEn : in STD_LOGIC;
    result: out std_logic_vector(31 downto 0);
    done : out STD_LOGIC);
end fdiv;

architecture fdiv_a of fdiv is

  component xilinx_fdiv
    port (
    a: in std_logic_vector(31 downto 0);
    b: in std_logic_vector(31 downto 0);
    operation_nd: in std_logic;
    clk: in std_logic;
    ce: in std_logic;
    result: out std_logic_vector(31 downto 0);
    rdy: out std_logic);
  end component;

begin
 
  xilinx_fdiv_i : xilinx_fdiv
    port map (
      a => a,
      b => b,
      operation_nd => go,
      clk => clk,
      result => result,
      ce => pipeEn,
      rdy => done);
end fdiv_a;

