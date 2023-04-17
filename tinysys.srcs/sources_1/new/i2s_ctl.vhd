-------------------------------------------------------------------------------
--                                                                 
--  COPYRIGHT (C) 2012, Digilent RO. All rights reserved
--                                                                  
-------------------------------------------------------------------------------
-- FILE NAME            : i2s_ctl.vhd
-- MODULE NAME          : I2S Control
-- AUTHOR               : Mihaita Nagy
-- AUTHOR'S EMAIL       : mihaita.nagy@digilent.ro
-------------------------------------------------------------------------------
-- REVISION HISTORY
-- VERSION  DATE         AUTHOR         DESCRIPTION
-- 1.0 	   2012-25-01   Mihaita Nagy   Created
-- 2.0      2012-02-04   Mihaita Nagy   Remade the i2s_transmitter.vhd and
--                                      i2s_receiver.vhd into one new module.
-- 3.0 	   2014-12-02   HegbeliC       Implemented edge detection for the
--                                      master mode and the division rate
--                                      for the different sampling rates
-------------------------------------------------------------------------------
-- KEYWORDS : I2S
-------------------------------------------------------------------------------
-- DESCRIPTION : This module implements the I2S transmitter and receiver 
--               interface, with a 32-bit Stereo data transmission. Parameter 
--               C_DATA_WIDTH sets the width of the data to be transmitted, 
--               with a maximum value of 32 bits. If a smaller width size is 
--               used (i.e. 24) than the remaining bits that needs to be 
--               transmitted to complete the 32-bit length, are automaticaly 
--               set to 0.
-------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

------------------------------------------------------------------------
-- Module Declaration
------------------------------------------------------------------------
entity i2s_ctl is
   generic (
      -- Width of one Slot (24/20/18/16-bit wide)
      C_DATA_WIDTH: integer := 16
   );
   port (
      CLK_I       : in  std_logic; -- System clock (100 MHz)
      RST_I       : in  std_logic; -- System reset		 
      EN_TX_I     : in  std_logic; -- Transmit enable
      EN_RX_I     : in  std_logic; -- Receive enable
		FS_I			: in  std_logic_vector(3 downto 0); -- Sampling rate 
		-- MM_I    		: in  std_logic; -- Audio controler Master Mode 
		D_L_I       : in  std_logic_vector(C_DATA_WIDTH-1 downto 0); -- Left channel data
      D_R_I       : in  std_logic_vector(C_DATA_WIDTH-1 downto 0); -- Right channel data
--      OE_L_O      : out std_logic; -- Left channel data output enable pulse
--      OE_R_O      : out std_logic; -- Right channel data output enable pulse
--      WE_L_O      : out std_logic; -- Left channel data write enable pulse
--      WE_R_O      : out std_logic; -- Right channel data write enable pulse     
      D_L_O       : out std_logic_vector(C_DATA_WIDTH-1 downto 0); -- Left channel data
      D_R_O       : out std_logic_vector(C_DATA_WIDTH-1 downto 0); -- Right channel data
      BCLK_O      : out std_logic; -- serial CLK
		LRCLK_O     : out std_logic; -- channel CLK
      SDATA_O     : out std_logic; -- Output serial data
      SDATA_I     : in  std_logic  -- Input serial data
   );
end i2s_ctl;

architecture Behavioral of i2s_ctl is

------------------------------------------------------------------------
-- Signal Declarations
------------------------------------------------------------------------
-- Counter for the clock divider
signal Cnt_Bclk               : integer range 0 to 31;

-- Counter for the L/R clock divider
signal Cnt_Lrclk              : integer range 0 to 31;

-- Rising and Falling edge impulses of the serial clock
signal BCLK_Fall, BCLK_Rise   : std_logic;
signal BCLK_Fall_int, BCLK_Rise_int   : std_logic;
--signal BCLK_Fall_shot, BCLK_Rise_shot : std_logic;

-- Synchronisation signals for Rising and Falling edge
signal Q1R, Q2R, Q3R : std_logic;
signal Q1F, Q2F, Q3F : std_logic;

-- Internal synchronous BCLK signal
signal BCLK_int               : std_logic;

-- Internal synchronous LRCLK signal
signal LRCLK_int              : std_logic;
signal LRCLK		            : std_logic;

--
signal Data_Out_int           : std_logic_vector(31 downto 0);

--
signal Data_In_int            : std_logic_vector(31 downto 0);

--
signal D_L_O_int              : std_logic_vector(C_DATA_WIDTH-1 downto 0);

--
signal D_R_O_int              : std_logic_vector(C_DATA_WIDTH-1 downto 0);

--Internal synchronous OE signals
signal OE_R_int, OE_L_int     : std_logic;

--Internal synchronous WE signals
signal WE_R_int, WE_L_int     : std_logic;

-- Division rate for the BCLK and LRCLK 
signal DIV_RATE : natural 		:= 4;

------------------------------------------------------------------------
-- Module Implementation
------------------------------------------------------------------------

begin

------------------------------------------------------------------------ 
-- Sampling frequency and data width decoder (DIV_RATE, C_DATA_WIDTH)
------------------------------------------------------------------------

	BIT_FS: process(CLK_I)
	begin
		if rising_edge(CLK_I) then
			case (FS_I) is
				when x"0" => DIV_RATE <= 24;
				when x"1" => DIV_RATE <= 16;
				when x"2" => DIV_RATE <= 12;
				when x"3" => DIV_RATE <= 8;
				when x"4" => DIV_RATE <= 6;
				when x"5" => DIV_RATE <= 4;
				when x"6" => DIV_RATE <= 2;
				when others => DIV_RATE <= 4;
			end case;
		end if;
	end process;

------------------------------------------------------------------------ 
-- Serial clock generator (BCLK_O, BCLK_Fall, BCLK_Rise)
------------------------------------------------------------------------
   SER_CLK: process(CLK_I)
   begin
      if rising_edge(CLK_I) then
         if RST_I = '1' then
            Cnt_Bclk <= 0;
            BCLK_int <= '0';
         elsif Cnt_Bclk = ((DIV_RATE/2)-1) then
            Cnt_Bclk <= 0;
            BCLK_int <= not BCLK_int;
         else
            Cnt_Bclk <= Cnt_Bclk + 1;
         end if;
      end if;
   end process SER_CLK;
   
   -- Rising and Falling edges when in Slave mode
   BCLK_Fall_int <= '1' when Cnt_Bclk = ((DIV_RATE/2)-1) and BCLK_int = '1' and (EN_RX_I = '1' or EN_TX_I = '1') else '0';
   BCLK_Rise_int <= '1' when Cnt_Bclk = ((DIV_RATE/2)-1) and BCLK_int = '0' and (EN_RX_I = '1' or EN_TX_I = '1') else '0';
	

   
	-- Falling edge selection with respect to Master Mode bit
	BCLK_Fall <= BCLK_Fall_int;
					 
	-- Risesing edge selection with respect to Master Mode bit				 
	BCLK_Rise <= BCLK_Rise_int;

   -- Serial clock output
   BCLK_O <= BCLK_int when EN_RX_I = '1' or EN_TX_I = '1' else '1';

------------------------------------------------------------------------ 
-- Left/Right clock generator (LRCLK_O, LRCLK_Pls)
------------------------------------------------------------------------
   LRCLK_GEN: process(CLK_I)
   begin
      if rising_edge(CLK_I) then
         if RST_I = '1' then
            Cnt_Lrclk <= 0;
            LRCLK <= '0'; -- Left channel active by default
         elsif BCLK_Fall = '1' then
            if Cnt_Lrclk = 31 then -- half of frame (64 bits)
               Cnt_Lrclk <= 0;
               LRCLK <= not LRCLK;
            else
               Cnt_Lrclk <= Cnt_Lrclk + 1;
            end if;
         end if;
      end if;
   end process LRCLK_GEN;
   
   -- L/R clock output
   LRCLK_O <= LRCLK when EN_TX_I = '1' or EN_RX_I = '1' else '0';
	LRCLK_int <= LRCLK;


------------------------------------------------------------------------ 
-- Load in paralled data, shift out serial data (SDATA_O)
------------------------------------------------------------------------
   SER_DATA_O: process(CLK_I)
   begin
      if rising_edge(CLK_I) then
         if RST_I = '1' then
            Data_Out_int(31) <= '0';
            Data_Out_int(30 downto 31-C_DATA_WIDTH) <= D_L_I; -- Left channel data by default
            Data_Out_int(30-C_DATA_WIDTH downto 0) <= (others => '0');
         elsif Cnt_Lrclk = 0 and BCLK_Rise = '1' then -- load par. data
            if LRCLK_int = '1' then
               Data_Out_int(31) <= '0';
               Data_Out_int(30 downto 31-C_DATA_WIDTH) <= D_R_I;
               Data_Out_int(30-C_DATA_WIDTH downto 0) <= (others => '0');
            else
               Data_Out_int(31) <= '0';
               Data_Out_int(30 downto 31-C_DATA_WIDTH) <= D_L_I;
               Data_Out_int(30-C_DATA_WIDTH downto 0) <= (others => '0');
            end if;
         elsif BCLK_Fall = '1' then -- shift out ser. data
            Data_Out_int <= Data_Out_int(30 downto 0) & '0';
         end if;
      end if;
   end process SER_DATA_O;
   
   -- Serial data output
   SDATA_O <= Data_Out_int(31) when EN_TX_I = '1' else '0';

------------------------------------------------------------------------ 
-- Shift in serial data, load out parallel data (SDATA_I)
------------------------------------------------------------------------
   SER_DATA_I: process(CLK_I)
   begin
      if rising_edge(CLK_I) then
         if RST_I = '1' then
            Data_In_int <= (others => '0');
            D_L_O_int <= (others => '0');
            D_R_O_int <= (others => '0');
         elsif Cnt_Lrclk = 0 and BCLK_Fall = '1' then -- load par. data
            if LRCLK_int = '1' then
               D_L_O_int <= Data_In_int(31 downto 32-C_DATA_WIDTH);
               Data_In_int <= (others => '0');
            else
               D_R_O_int <= Data_In_int(31 downto 32-C_DATA_WIDTH);
               Data_In_int <= (others => '0');
            end if;
         elsif BCLK_Rise = '1' then -- shift in ser. data
            Data_In_int <= Data_In_int(30 downto 0) & SDATA_I;
         end if;
      end if;
   end process SER_DATA_I;
   
   D_L_O <= D_L_O_int;
   D_R_O <= D_R_O_int;

-------------------------------------------------------------------------- 
---- Output Enable signals (for FIFO)
--------------------------------------------------------------------------
--   OE_GEN: process(CLK_I)
--   begin
--      if rising_edge(CLK_I) then
--         if Cnt_Lrclk = 31 and BCLK_Fall = '1' then
--            if LRCLK_int = '1' then -- Right channel
--               OE_R_int <= '1';
--            else -- Left channel
--               OE_L_int <= '1';
--            end if;
--         else
--            OE_R_int <= '0';
--            OE_L_int <= '0';
--         end if;
--      end if;
--   end process OE_GEN;
   
--   OE_R_O <= OE_R_int when EN_TX_I = '1' else '0';
--   OE_L_O <= OE_L_int when EN_TX_I = '1' else '0';

-------------------------------------------------------------------------- 
---- Write Enable signals (for FIFO)
--------------------------------------------------------------------------
--   WE_GEN: process(CLK_I)
--   begin
--      if rising_edge(CLK_I) then
--         if Cnt_Lrclk = 1 and BCLK_Rise = '1' then
--            if LRCLK_int = '1' then -- Right channel
--               WE_R_int <= '1';
--            else -- Left channel
--               WE_L_int <= '1';
--            end if;
--         else
--            WE_R_int <= '0';
--            WE_L_int <= '0';
--         end if;
--      end if;
--   end process WE_GEN;
   
--   WE_R_O <= WE_R_int when EN_RX_I = '1' else '0';
--   WE_L_O <= WE_L_int when EN_RX_I = '1' else '0';

end Behavioral;

