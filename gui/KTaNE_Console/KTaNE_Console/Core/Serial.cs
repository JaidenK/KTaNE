using System;
using System.IO.Ports;
using System.Text;
using System.Threading;

namespace KTaNE_Console.Core
{
    public class Serial
    {
        private readonly int PACKET_LENGTH_OFFSET = 2;
        public static readonly byte SYNC_BYTE = 0xA5;

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

        private enum RxPacketState
        {
            SearchingForSync1,
            SearchingForSync2,
            ReadingLength,
            AcquiringDataBytes
        }

        private void SerialThread()
        {
            byte[] buf = new byte[1024];
            byte[] packetBuf = new byte[256]; // only needs to be enough to hold 1 packet
            const int MaxPacketSize = 100;
            int iPacketBuf = 0;
            int nBytesRead = 0;
            int iSync = -1;
            string newChars = "";
            byte packetLength = 0;
            RxPacketState state = RxPacketState.SearchingForSync1;


            while (true)
            {
                if (!_port.IsOpen)
                {
                    Thread.Sleep(100);
                    continue;
                }

                if (_port.BytesToRead == 0)
                {
                    Thread.Sleep(100);
                    continue;
                }

                lock (_lock)
                {
                    nBytesRead = _port.Read(buf, 0, buf.Length);
                }

                // Using string builder and writing the result should
                // be less CPU intense then writing one character at a time.
                StringBuilder sb = new StringBuilder(128);
                for (int i = 0; i < nBytesRead; i++)
                {
                    byte newByte = buf[i];
                    switch (state)
                    {
                        case RxPacketState.SearchingForSync1:
                            {
                                iPacketBuf = 0;
                                if (newByte == SYNC_BYTE)
                                {
                                    state = RxPacketState.SearchingForSync2;
                                }
                                else
                                {
                                    sb.Append((char)newByte);
                                }
                                break;
                            }
                        case RxPacketState.SearchingForSync2:
                            {
                                if (newByte == SYNC_BYTE)
                                {
                                    state = RxPacketState.ReadingLength;
                                }
                                else
                                {
                                    sb.Append((char)newByte);
                                    state = RxPacketState.SearchingForSync1;
                                }
                                break;
                            }
                        case RxPacketState.ReadingLength:
                            {
                                packetLength = newByte;
                                state = RxPacketState.AcquiringDataBytes;
                                break;
                            }
                        case RxPacketState.AcquiringDataBytes:
                            {
                                // Do nothing (byte is always put in buf)
                                break;
                            }
                        default:
                            throw new Exception("Unhandled RxPacketState: " + state.ToString());
                            break;
                    }

                    // Store the date
                    packetBuf[iPacketBuf++] = newByte;

                    if (((byte)iPacketBuf == packetLength) && (state == RxPacketState.AcquiringDataBytes))
                    {
                        state = RxPacketState.SearchingForSync1;
                        // Check CRC
                        //crc.EvaluateCRC()

                        // We have a packet. Put it in the packet array
                        var args = new SerialPacketReceivedEventArgs
                        {
                            packet = new byte[packetLength]
                        };
                        Array.Copy(packetBuf, 0, args.packet, 0, packetLength);
                        PacketReceived.Invoke(this, args);
                    }
                }
                if (sb.Length > 0)
                {
                    Write(sb.ToString());
                }
            }
        }

        private void _port_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            Write($"Serial Error:\n" + e.EventType.ToString() + "\n");
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

        public void Write(byte[] bytes)
        {
            lock(_lock)
            {
                if(_port.IsOpen)
                    _port.Write(bytes, 0, bytes.Length);
            }
        }
    }
}
