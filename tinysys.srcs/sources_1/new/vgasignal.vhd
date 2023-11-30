LIBRARY IEEE;
LIBRARY UNISIM;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;
USE UNISIM.VCOMPONENTS.all;

ENTITY vgatimer IS
  PORT( clk_i   : IN  std_logic;
  		rst_i   : IN  std_logic;
        hsync_o : OUT std_logic;
        vsync_o : OUT std_logic;
        counter_x : OUT std_logic_vector(11 downto 0);
        counter_y : OUT std_logic_vector(11 downto 0);
  SIGNAL vsynctrigger_o : OUT std_logic;
  SIGNAL vsynccounter : OUT std_logic_vector(31 downto 0) );
END ENTITY vgatimer;

ARCHITECTURE vgatimer_a OF vgatimer IS
---- 1920x1080 (1080p)
--	constant C_RES_X  : integer := 1920;
--	constant C_RES_Y  : integer := 1080;
--	constant C_HFRONT : integer := 88;
--	constant C_HSYNC  : integer := 44;
--	constant C_HBACK  : integer := 148;
--	constant C_VFRONT : integer := 4;
--	constant C_VSYNC  : integer := 5;
--	constant C_VBACK  : integer := 36;

---- 1280x720 (720p)
--	constant C_RES_X  : integer := 1280;
--	constant C_RES_Y  : integer := 720;  
--	constant C_HFRONT : integer := 110;
--	constant C_HSYNC  : integer := 40;
--	constant C_HBACK  : integer := 220;
--	constant C_VFRONT : integer := 5;
--	constant C_VSYNC  : integer := 5;
--	constant C_VBACK  : integer := 20;

 ---- 640x480 (480p)
	constant C_RES_X  : integer := 640;
	constant C_RES_Y  : integer := 480;  
	constant C_HFRONT : integer := 16; 
	constant C_HSYNC  : integer := 96;
	constant C_HBACK  : integer := 48;
	constant C_VFRONT : integer := 10;
	constant C_VSYNC  : integer := 2;
	constant C_VBACK  : integer := 33;

	constant C_HBLANK  : integer := C_HFRONT + C_HSYNC + C_HBACK;
	constant C_VBLANK  : integer := C_VFRONT + C_VSYNC + C_VBACK;
	constant C_TOTAL_X : integer := C_RES_X + C_HBLANK;
	constant C_TOTAL_Y : integer := C_RES_Y + C_VBLANK;

	signal x: unsigned(11 downto 0) := (OTHERS => '0');
	signal y: unsigned(11 downto 0) := (OTHERS => '0');
	signal vscnt : unsigned(31 downto 0) := (OTHERS => '0');
BEGIN

  counter_x <= std_logic_vector(x);
  counter_y <= std_logic_vector(y);
  vsynccounter <= std_logic_vector(vscnt);

  timing : PROCESS(clk_i)
  BEGIN
    IF rising_edge(clk_i) THEN
        vsynctrigger_o <= '0';
      
        IF (x = C_RES_X+C_HFRONT - 1) THEN
            hsync_o <= '1';
        ELSIF (x = C_RES_X + C_HFRONT + C_HSYNC - 1) THEN
            hsync_o <= '0';
        END IF;

        IF (x = C_TOTAL_X - 1) THEN 
            x <= (OTHERS => '0');
            IF (y = C_RES_Y + C_VFRONT - 1) THEN
                vsync_o  <= '1';
                vscnt <= vscnt + 1;
                vsynctrigger_o <= '1';
            ELSIF (y = C_RES_Y + C_VFRONT + C_VSYNC - 1) THEN
                vsync_o  <= '0';
            END IF;
            
            IF (y = C_TOTAL_Y - 1) THEN
                y <= (OTHERS => '0');
            ELSE
                y <= y + 1;
            END IF;
        ELSE
            x <= x + 1;
        END IF;    
    END IF;
  END PROCESS;

END ARCHITECTURE vgatimer_a;