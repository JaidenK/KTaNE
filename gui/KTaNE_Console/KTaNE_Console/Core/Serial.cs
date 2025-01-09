using KTaNE_Console.Model;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Windows.Documents;

namespace KTaNE_Console.Core
{
    public class Serial
    {
        public static readonly byte SYNC_BYTE = 0xA5;

        private readonly object _lock = new object();
        private readonly object _queuelock = new object();
        private SerialPort _port { get; set; }
        public string PortString => $"{_port?.PortName ?? "NULL"}@{_port?.BaudRate ?? 0}";

        private Thread _thread;

        public event EventHandler<SerialTextReceivedEventArgs> TextReceived;
        public event EventHandler<SerialPacketReceivedEventArgs> PacketReceived;
        public event EventHandler<SerialPacketSentEventArgs> PacketSent;

        byte rx_seqCnt = 0;
        byte tx_seqCnt = 0;

        Queue<byte[]> TxQueue = new Queue<byte[]>();

        public void LogString(string s)
        {
            TextReceived?.Invoke(this, new SerialTextReceivedEventArgs(s));
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
            int iPacketBuf = 0;
            int nBytesRead = 0;
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
                    }

                    // Store the date
                    packetBuf[iPacketBuf++] = newByte;

                    if (((byte)iPacketBuf == packetLength) && (state == RxPacketState.AcquiringDataBytes))
                    {
                        state = RxPacketState.SearchingForSync1;

                        rx_seqCnt = packetBuf[3];
                        // Check CRC
                        //crc.EvaluateCRC()

                        // We have a packet. Put it in the packet array
                        var args = new SerialPacketReceivedEventArgs
                        {
                            packet = new byte[packetLength]
                        };
                        Array.Copy(packetBuf, 0, args.packet, 0, packetLength);
                        try
                        {
                            PacketReceived?.Invoke(this, args);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine(ex.ToString());
                            //throw;
                        }
                    }
                }
                if (sb.Length > 0)
                {
                    LogString(sb.ToString());
                }

                lock(_queuelock)
                {
                    if(TxQueue.Count > 0)
                    {
                        // TODO Check if the last sent packet has been ACK'd
                        // Or just throttle the transmission rate...
                        var bytes = TxQueue.Dequeue();
                        Write(bytes);
                    }
                }
            }
        }

        private void _port_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            LogString($"Serial Error:\n" + e.EventType.ToString() + "\n");
        }

        public void Connect(string PortName, int BaudRate)
        {
            if(PortName is null)
            {
                return;
            }

            lock (_lock)
            {
                _port.Close();
                _port.PortName = PortName;
                _port.BaudRate = BaudRate;
                LogString($"Connecting to {PortString}... ");
                try
                {
                    _port.Open();
                    _port.DiscardInBuffer();
                }
                catch (Exception ex)
                {
                    LogString($"\nFailed to open {PortString}:\n" + ex.ToString() + "\n");
                    return;
                }
                LogString($"Connected!\n");
            }
        }

        // Making this private to force users into complying with the protocol
        private void Write(byte[] bytes)
        {
            lock (_lock)
            {
                if (_port.IsOpen)
                {
                    _port.Write(bytes, 0, bytes.Length);
                }
            }
        }

        public byte[] SendCommand(byte nResponseBytes, byte address, byte command, UInt32 argument)
        {
            byte[] bytes = new byte[5];
            bytes[0] = command;
            Array.Copy(BitConverter.GetBytes(argument), 0, bytes, 1, 4);

            return SendCommand(nResponseBytes, address, bytes);
        }

        public byte[] SendCommand(byte nResponseBytes, byte address, byte[] i2c_bytes)
        {

            byte[] bytes = new byte[13 + i2c_bytes.Length];
            bytes[0] = Serial.SYNC_BYTE;
            bytes[1] = Serial.SYNC_BYTE;
            bytes[2] = (byte)bytes.Length;
            bytes[3] = tx_seqCnt;
            bytes[4] = rx_seqCnt;
            bytes[9] = nResponseBytes;
            bytes[10] = (byte)address;
            for (int i = 0; i < i2c_bytes.Length; i++)
            {
                bytes[UartPacket.DATA_START + i] = i2c_bytes[i];
            }

            // Calculate CRC
            // ... 

            //Write(bytes);
            // Enqueued bytes will automatically begin sending in the serial thread
            lock(_queuelock)
            {
                TxQueue.Enqueue(bytes);
            }

            var args = new SerialPacketSentEventArgs
            {
                packet = new byte[bytes.Length]
            };
            Array.Copy(bytes, 0, args.packet, 0, bytes.Length);

            PacketSent?.Invoke(this, args);
            
            tx_seqCnt++;

            return bytes;
        }

        //TODO
        public void SetRegister(byte i2c_address, byte register_address, byte value)
        {
            throw new NotImplementedException();
        }
    }
}
