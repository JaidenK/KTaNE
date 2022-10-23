namespace KTaNE_Console.Model
{
    public class UartPacket
    {
        // Addresses
        public const byte TIMER_ADDRESS = 0x01;

        // Array Indices
        public const int DATA_START = 11;

        #region Command IDs

        // Initialization commands
        public const byte REQUEST_CONFIG = 0x15;
        public const byte RESET = 0x16;

        // Configuration commands
        public const byte FLASH_LED = 0x50;
        public const byte SET_SERIAL_NO = 0x51;
        public const byte SET_N_BATTERIES = 0x52;
        public const byte SET_INDICATOR = 0x53;
        public const byte SET_PORT = 0x54;
        public const byte SET_TIME_LIMIT = 0x55;
        #endregion

        public byte seqCnt { get; set; }
        public byte seqCntEcho { get; set; }
        public byte nResponseBytes { get; set; }
        public byte address { get; set; }
        public byte[] i2c_bytes { get; set; }
        public byte[] crc { get; set; }

        public static UartPacket FromFullPacket(byte[] buf)
        {

            var length = buf[2];
            var dataLength = length - (DATA_START + 2);

            var packet = new UartPacket
            {
                seqCnt = buf[3],
                seqCntEcho = buf[4],
                nResponseBytes = buf[9],
                address = buf[10],
                i2c_bytes = new byte[dataLength],
                crc = new byte[] { buf[length - 2], buf[length - 1] }
            };

            for(int i = 0; i < dataLength; i++)
            {
                packet.i2c_bytes[i] = buf[DATA_START + i];
            }

            return packet;
        }

        public override string ToString()
        {
            string s = $"{seqCnt} {seqCntEcho} | {nResponseBytes} | 0x{address:X2} | ";
            foreach(var b in i2c_bytes)
            {
                s += $"{b:X2} ";
            }
            s += $"| 0x{crc[0]:X2} 0x{crc[1]:X2}";
            return s;
        }
    }
}
