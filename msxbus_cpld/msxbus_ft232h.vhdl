library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Z80_FT232H_Interface is
    Port (
        -- Z80 Bus Signals
        Z80_ADDR     : in  std_logic_vector(15 downto 0);
        Z80_DATA     : inout std_logic_vector(7 downto 0);
        Z80_RD       : in  std_logic;
        Z80_WR       : in  std_logic;
        Z80_MREQ     : in  std_logic;
        Z80_IORQ     : in  std_logic;
        Z80_WAIT     : out std_logic;
        
        -- FT232H Interface Signals
        FT232H_DATA  : inout std_logic_vector(7 downto 0);
        FT232H_RD    : out std_logic;
        FT232H_WR    : out std_logic;
        FT232H_TXE   : in  std_logic;
        FT232H_RXF   : in  std_logic;
        
        -- Control Signals
        CLK          : in  std_logic;
        RESET        : in  std_logic
    );
end Z80_FT232H_Interface;

architecture Behavioral of Z80_FT232H_Interface is
    type state_type is (IDLE, DECODE, WAIT_FOR_HOST, SEND_CMD, RECEIVE_DATA, SEND_DATA, WAIT_STATE);
    signal state : state_type := IDLE;
    
    signal cmd_reg       : std_logic_vector(7 downto 0) := (others => '0');
    signal addr_reg      : std_logic_vector(15 downto 0) := (others => '0');
    signal data_reg      : std_logic_vector(7 downto 0) := (others => '0');
    signal tx_buf        : std_logic_vector(7 downto 0) := (others => '0');
    signal rx_buf        : std_logic_vector(7 downto 0) := (others => '0');
    signal wait_req      : std_logic := '0';
    
begin
    Z80_WAIT <= wait_req;
    
    process(CLK, RESET)
    begin
        if RESET = '1' then
            state <= IDLE;
            wait_req <= '0';
            FT232H_RD <= '1';
            FT232H_WR <= '1';
            Z80_DATA <= (others => 'Z');
            FT232H_DATA <= (others => 'Z');
            
        elsif rising_edge(CLK) then
            case state is
                when IDLE =>
                    if Z80_MREQ = '0' or Z80_IORQ = '0' then
                        -- Latch address and data
                        addr_reg <= Z80_ADDR;
                        if Z80_WR = '0' then
                            data_reg <= Z80_DATA;
                        end if;
                        
                        -- Determine command
                        if Z80_MREQ = '0' then
                            if Z80_RD = '0' then
                                cmd_reg <= x"01"; -- MREQ_RD
                            else
                                cmd_reg <= x"02"; -- MREQ_WR
                            end if;
                        else -- IORQ
                            if Z80_RD = '0' then
                                cmd_reg <= x"03"; -- IORQ_RD
                            else
                                cmd_reg <= x"04"; -- IORQ_WR
                            end if;
                        end if;
                        
                        state <= DECODE;
                        wait_req <= '1'; -- Assert WAIT
                    end if;
                    
                when DECODE =>
                    -- Prepare to send command to FT232H
                    if FT232H_TXE = '0' then -- FT232H can accept data
                        state <= SEND_CMD;
                    end if;
                    
                when SEND_CMD =>
                    -- Send command byte
                    FT232H_WR <= '0';
                    FT232H_DATA <= cmd_reg;
                    state <= WAIT_STATE;
                    
                when WAIT_STATE =>
                    FT232H_WR <= '1';
                    -- Next send address (high byte)
                    tx_buf <= addr_reg(15 downto 8);
                    state <= SEND_ADDR_H;
                    
                when SEND_ADDR_H =>
                    if FT232H_TXE = '0' then
                        FT232H_WR <= '0';
                        FT232H_DATA <= tx_buf;
                        state <= WAIT_STATE2;
                    end if;
                    
                when WAIT_STATE2 =>
                    FT232H_WR <= '1';
                    -- Next send address (low byte)
                    tx_buf <= addr_reg(7 downto 0);
                    state <= SEND_ADDR_L;
                    
                when SEND_ADDR_L =>
                    if FT232H_TXE = '0' then
                        FT232H_WR <= '0';
                        FT232H_DATA <= tx_buf;
                        state <= WAIT_STATE3;
                    end if;
                    
                when WAIT_STATE3 =>
                    FT232H_WR <= '1';
                    if cmd_reg = x"02" or cmd_reg = x"04" then
                        -- For write commands, send data
                        tx_buf <= data_reg;
                        state <= SEND_DATA;
                    else
                        -- For read commands, wait for response
                        state <= WAIT_FOR_HOST;
                    end if;
                    
                when SEND_DATA =>
                    if FT232H_TXE = '0' then
                        FT232H_WR <= '0';
                        FT232H_DATA <= tx_buf;
                        state <= WAIT_STATE4;
                    end if;
                    
                when WAIT_STATE4 =>
                    FT232H_WR <= '1';
                    state <= WAIT_FOR_HOST;
                    
                when WAIT_FOR_HOST =>
                    if FT232H_RXF = '0' then -- Data available from FT232H
                        FT232H_RD <= '0';
                        state <= RECEIVE_DATA;
                    end if;
                    
                when RECEIVE_DATA =>
                    rx_buf <= FT232H_DATA;
                    FT232H_RD <= '1';
                    
                    if rx_buf = x"00" then -- Data ready
                        if cmd_reg = x"01" or cmd_reg = x"03" then
                            -- For read commands, get the data
                            state <= GET_READ_DATA;
                        else
                            -- For write commands, done
                            wait_req <= '0';
                            state <= IDLE;
                        end if;
                    elsif rx_buf = x"FF" then
                        -- Host not ready, keep waiting
                        state <= WAIT_FOR_HOST;
                    end if;
                    
                when GET_READ_DATA =>
                    if FT232H_RXF = '0' then
                        FT232H_RD <= '0';
                        state <= FINISH_READ;
                    end if;
                    
                when FINISH_READ =>
                    Z80_DATA <= FT232H_DATA;
                    FT232H_RD <= '1';
                    wait_req <= '0';
                    state <= IDLE;
                    
                when others =>
                    state <= IDLE;
            end case;
        end if;
    end process;
    
end Behavioral;