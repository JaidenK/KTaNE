using System;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;

namespace KTaNE_Console.Core
{
    public class Serial
    {
        private readonly int PACKET_LENGTH_OFFSET = 2;
        private readonly byte SYNC_BYTE = 0xA5;

        private Serial _instance;

        public Serial Instance
        {
            get
            {
                if (_instance is null)
                {
                    _instance = new Serial();
                }
                return _instance;
            }
        }
        
        private readonly object _lock = new object();
        private SerialPort _port { get; set; }
        public string PortString => $"{_port?.PortName ?? "NULL"}@{_port?.BaudRate ?? 0}";

        private Thread _thread;

        public event EventHandler<SerialTextReceivedEventArgs> TextReceived;
        public event EventHandler<SerialPacketReceivedEventArgs> PacketReceived;

        public void Write(string s)
        {
            TextReceived.Invoke(this, new SerialTextReceivedEventArgs(s));
        }

        public Serial()
        {
            _port = new SerialPort
            {
                ReadTimeout = 50,
                WriteTimeout = 50
            };
            _port.ErrorReceived += _port_ErrorReceived;

            _thread = new Thread(new ThreadStart(SerialThread))
            {
                IsBackground = true
            };
            _thread.Start();
        }

        private void SerialThread()
        {
            byte[] buf = new byte[1024];
            byte[] packetBuf = new byte[256]; // only needs to be enough to hold 1 packet
            int ibuf = 0;
            int iSync = -1;
            string newChars = "";

            while(true)
            {
                if(!_port.IsOpen)
                {
                    Thread.Sleep(100);
                    continue;
                }

                if(_port.BytesToRead == 0)
                {
                    Thread.Sleep(100);
                    continue;
                }

                lock(_lock)
                {
                    ibuf += _port.Read(buf, ibuf, buf.Length - ibuf);
                    //newChars = _port.ReadExisting();
                }

                //Write(newChars);

                // Copy incoming characters to the buffer
                //Array.Copy(newChars.ToCharArray(), 0, buf, ibuf, newChars.Length);
                //ibuf += newChars.Length;

                // If the buffer doesn't contain the sync, clear it.
                // This could fail if we get one byte before the next.
                StringBuilder sb = new StringBuilder(128);
                for(int i = 0; i < buf.Length-1 && iSync < 0; i++)
                {
                    if(buf[i] == SYNC_BYTE && buf[i+1] == SYNC_BYTE)
                    {
                        iSync = i;
                        break; // Break not necessary because of the iSync < 0
                    }
                    else
                    {
                        sb.Append((char)buf[i]);
                    }
                }
                if (sb.Length > 0)
                    Write(sb.ToString());

                // Clear the array if we didn't find the sync. But also
                // check that the last byte isn't potentially the sync just
                // in case it came in on the edge.
                if(iSync < 0 && buf[ibuf-1] != SYNC_BYTE)
                {
                    // We're probably fine just setting the index. Actually
                    // clearing the array probably isn't necessary.
                    Array.Clear(buf, 0, buf.Length);
                    ibuf = 0;
                }
                else if(iSync >= 0)
                {
                    // Check if we've received a whole packet.
                    // packet length is stored at byte 2
                    byte packetLength = (byte)buf[iSync + PACKET_LENGTH_OFFSET];
                    if(ibuf - iSync >= packetLength)
                    {
                        // We have a packet. Put it in the packet array
                        var args = new SerialPacketReceivedEventArgs
                        {
                            packet = new byte[packetLength]
                        };
                        Array.Copy(buf, iSync, args.packet, 0, packetLength);
                        PacketReceived.Invoke(this, args);

                        // Shift over all the leftover data in the buffer. 
                        int nExtraBytes = ibuf - (iSync + packetLength);
                        Array.Copy(buf, iSync+packetLength, buf, 0, nExtraBytes);
                        ibuf = nExtraBytes;
                        iSync = -1;
                    }
                }


                

            }
        }

        private void _port_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            Write($"Serial Error:\n"+e.EventType.ToString()+"\n");
        }

        public void Connect(string PortName, int BaudRate)
        {

            lock (_lock)
            {
                _port.Close();
                _port.PortName = PortName;
                _port.BaudRate = BaudRate;
                Write($"Connecting to {PortString}... ");
                try
                {
                    _port.Open();
                    _port.DiscardInBuffer();
                }
                catch (Exception ex)
                {
                    Write($"\nFailed to open {PortString}:\n" + ex.ToString() + "\n");
                    return;
                }
                Write($"Connected!\n");
            }
        }
    }
}
