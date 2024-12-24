using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net;
using System.Text;
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

namespace KTaNE_Console.Modules
{
    /// <summary>
    /// Interaction logic for ButtonModulePanel.xaml
    /// </summary>
    public partial class ButtonModulePanel : UserControl, INotifyPropertyChanged, IModulePanel
    {
        IModule module;
        public string LabelFeedback
        {
            get
            {
                if (module is null)
                    return "NULL";
                return Encoding.ASCII.GetString(module.Eeprom.bytes, EEPROM.BUTTON_LABEL, 16).Trim();
            }
        }
        public string LabelInput { get; set; } = "";
        public string ButtonColorFeedback
        {
            get
            {
                if (module is null)
                    return "NULL";
                return Encoding.ASCII.GetString(module.Eeprom.bytes, EEPROM.BUTTON_BTN_COLOR, 16).Trim();
            }
        }
        public string ButtonColorInput { get; set; } = "";
        public string StripColorFeedback
        {
            get
            {
                if (module is null)
                    return "NULL";
                return Encoding.ASCII.GetString(module.Eeprom.bytes, EEPROM.BUTTON_STRIP_COLOR, 16).Trim();
            }
        }

        public string NBatteries
        {
            get
            {
                if (module is null)
                    return "NULL";
                return (module.Eeprom.bytes[EEPROM.AA_BATTERIES] + module.Eeprom.bytes[EEPROM.D_BATTERIES]).ToString();
            }
        }

        public string LitIndicators
        {
            get
            {
                if (module is null)
                    return "NULL";

                string s = "";
                for (int i = 0; ; i++)
                {
                    int addr = EEPROM.INDICATORS + 4 * i;
                    var str = Encoding.ASCII.GetString(module.Eeprom.bytes, addr, 4);
                    if (str.Length == 4)
                    {
                        if (str[0] == '+' || str[0] == '-')
                        {
                            if (str[0] == '+')
                            {
                                s += str.Substring(1, 3) + ", ";
                            }
                            continue;
                        }
                    }
                    break;
                }
                return s;
            }
        }

        public string StripColorInput { get; set; } = "";

        public ButtonModulePanel(IModule module)
        {
            this.module = module;
            this.DataContext = this;

            InitializeComponent();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public List<byte[]> BuildSetEEPROMPackets()
        {
            if (module is null) return null;

            var list = new List<byte[]>();

            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_LABEL, Encoding.ASCII.GetBytes(LabelInput.ToUpper().Trim())));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_BTN_COLOR, Encoding.ASCII.GetBytes(ButtonColorInput.ToUpper().Trim())));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_STRIP_COLOR, Encoding.ASCII.GetBytes(StripColorInput.ToUpper().Trim())));

            return list;
        }
    }
}
