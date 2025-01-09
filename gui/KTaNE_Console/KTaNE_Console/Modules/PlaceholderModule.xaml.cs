using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.ViewModel;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace KTaNE_Console.Modules
{
    /// <summary>
    /// Interaction logic for PlaceholderModule.xaml
    /// </summary>
    public partial class PlaceholderModule : UserControl, IModule, INotifyPropertyChanged
    {

        public event PropertyChangedEventHandler PropertyChanged;
        public byte BusAddress { get; set; }

        public string ModelID { get; set; }

        public SolvedState State { get; set; }

        private byte _Status = 0;
        public byte Status
        {
            get => _Status; 
            set
            {
                if(value != _Status)
                {
                    _Status = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Status"));
                }
            }
        }

        public IModulePanel ModulePanel { get; set; } = new UnrecognizedModulePanel();

        private string moduleName = "<NULL>";
        public string ModuleName
        {
            get
            {
                moduleName = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_NAME, 16).Trim();
                return moduleName;
            }
            set => moduleName = value;
        }
        private string moduleSerialNo = "<NULL>";
        public string ModuleSerialNo
        {
            get
            {
                moduleSerialNo = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_SERIAL, 16).Trim();
                return moduleSerialNo;
            }
            set => moduleSerialNo = value;
        }

        private string moduleVersion = "<NULL>";
        public string ModuleVersion
        {
            get
            {
                moduleVersion = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_DATE, 16).Trim();
                return moduleVersion;
            }
            set => moduleVersion = value;
        }

        public EEPROM Eeprom { get; set; } = new EEPROM();

        public List<byte[]> BuildGetEEPROMPackets(ushort eeprom_address, UInt16 length)
        {
            int remainingLength = length;
            int i = 0;
            List<byte[]> retval = new List<byte[]>();
            while (remainingLength > 0)
            {
                var current_address = (ushort)(eeprom_address + (0x10 * i));
                i++;
                var eeprom_addr_bytes = BitConverter.GetBytes(current_address);
                byte currentLength = (byte)(remainingLength > 16 ? 16 : remainingLength);
                remainingLength -= 16;

                byte[] bytes = new byte[4];
                
                bytes[0] = (byte)CommandID.GET_EEPROM;
                bytes[1] = eeprom_addr_bytes[0]; // EEPROM Addresss lower byte
                bytes[2] = eeprom_addr_bytes[1]; // EEPROM Addresss upper byte
                bytes[3] = currentLength; // number of bytes

                retval.Add(bytes);
            }

            return retval;
        }
        public void UpdatePanel()
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                var name = ModuleName.Trim().ToUpper();
                switch (name)
                {
                    case "KEYPADS":
                        ModulePanel = new KeypadsModulePanel(this);
                        break;
                    case "THE BUTTON":
                        ModulePanel = new ButtonModulePanel(this);
                        break;
                    case "SKETCHPAD":
                        ModulePanel = new SketchpadModulePanel();
                        break;
                    case "COMP. WIRES":
                        ModulePanel = new ComplicatedWiresPanel(this);
                        break;
                    default:
                        ModulePanel = new UnrecognizedModulePanel();
                        break;
                }
            }), DispatcherPriority.DataBind);
        }

        private Serial serial;
        private IConsoleWriter CW;
        public RelayCommand BlinkCmd { get; set; }
        public RelayCommand ReadEEPROMCmd { get; set; }
        public RelayCommand ApplyEEPROMCmd { get; set; }
        private Queue<byte[]> TxQueue { get; set; } = new Queue<byte[]>();
        public PlaceholderModule(byte addr, Serial serial, IConsoleWriter ConsoleWriter)
        {
            InitializeComponent();
            this.DataContext = this;
            this.serial = serial;
            this.CW = ConsoleWriter;
            this.BusAddress = addr;
            serial.PacketReceived += Serial_PacketReceived;

            ReadEEPROMCmd = new RelayCommand((o) =>
            {
                try
                {
                    BuildGetEEPROMPackets(0, 512).ForEach(x =>
                    {
                        serial.SendCommand(20, BusAddress, x); // Why 20??
                    });
                }
                catch (Exception ex)
                {
                    CW.ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            BlinkCmd = new RelayCommand((o) =>
            {
                Task.Run(() =>
                {
                    for(int i = 0; i < 3; i++)
                    {
                        serial.SendCommand(0, BusAddress, new byte[] { (byte)Registers.REG_CTRL, (byte)0x0C });
                        Thread.Sleep(100);
                        serial.SendCommand(0, BusAddress, new byte[] { (byte)Registers.REG_CTRL, (byte)0x0C });
                        Thread.Sleep(100);
                    }
                });
            });

            ApplyEEPROMCmd = new RelayCommand((o) =>
            {
                try
                {
                    Task.Run((Action)(() =>
                    {
                        CW.ConsoleWrite($"Configuring {this.ModuleName}..." + Environment.NewLine);
                        var pkts = ModulePanel.BuildSetEEPROMPackets();
                        if(pkts != null)
                        {
                            pkts.ForEach(x =>
                            {
                                serial.SendCommand(0, BusAddress, x);
                                //Task.Delay(200); // Delay removed because reliability needs to be part of the Serial interface code
                            });
                            CW.ConsoleWrite("Done." + Environment.NewLine);
                        }
                        else
                        {
                            CW.ConsoleWrite("No configuration needed." + Environment.NewLine);
                        }
                    }));
                }
                catch (Exception ex)
                {
                    CW.ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            Task.Run(() =>
            {
                Thread.Sleep(1000);
                ReadEEPROMCmd.Execute(this);
            });
        }
        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            //if (TxQueue.Count > 0)
            //    serial.Write(TxQueue.Dequeue());

            var pkt = UartPacket.FromFullPacket(e.packet);

            if (((byte)(pkt.address & 0b01111111)) != BusAddress)
                return;

            switch (pkt.Command)
            {
                case CommandID.GET_EEPROM:
                    int eeprom_address = BitConverter.ToInt16(pkt.i2c_bytes, 1);
                    int length = pkt.i2c_bytes[3];
                    var bytes = new byte[length];
                    Array.Copy(pkt.i2c_bytes, 4, bytes, 0, length);
                    Eeprom.Update(eeprom_address, bytes);
                    UpdatePanel(); // This makes a new object every single time we read back the eeprom? I think that's why the forms reset
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
                    break;
                default:
                    break;
            }
        }
    }
}
