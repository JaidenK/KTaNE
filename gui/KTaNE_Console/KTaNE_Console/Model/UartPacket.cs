namespace KTaNE_Console.Model
{
    public class UartPacket
    {
        public static readonly int DATA_START = 11;

        public byte seqCnt { get; set; }
        public byte seqCntEcho { get; set; }
        public byte nResponseBytes { get; set; }
        public byte address { get; set; }
        public byte[] i2c_bytes { get; set; }
        public byte[] crc { get; set; }

        public static UartPacket FromFullPacket(byte[] buf)
        {

            var length = buf[2];
            var dataLength = length - (DATA_START + 1 + 2);

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
