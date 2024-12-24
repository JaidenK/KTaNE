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
                switch(module.Eeprom.bytes[EEPROM.BUTTON_LABEL])
                {
                    case 0: return "(other)";
                    case 1: return "ABORT";
                    case 2: return "DETONATE";
                    case 3: return "HOLD";
                }
                return "NULL";
            }
        }
        public int LabelInput { get; set; } = 0; // Why does this get reset when EEPROM is read?
        public string ButtonColorFeedback
        {
            get
            {
                if (module is null)
                    return "NULL";
                switch (module.Eeprom.bytes[EEPROM.BUTTON_BTN_COLOR])
                {
                    case 0: return "(other)";
                    case 1: return "Blue";
                    case 2: return "White";
                    case 3: return "Yellow";
                    case 4: return "Red";
                }
                return "NULL";
            }
        }
        public int ButtonColorInput { get; set; } = 0;
        public string StripColorFeedback
        {
            get
            {
                if (module is null)
                    return "NULL";
                switch (module.Eeprom.bytes[EEPROM.BUTTON_STRIP_COLOR])
                {
                    case 0: return "(random)";
                    case 1: return "Red";
                    case 2: return "Yellow";
                    case 3: return "Green";
                    case 4: return "Cyan";
                    case 5: return "Blue";
                    case 6: return "Magenta";
                    case 7: return "White";
                }
                return "NULL";
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
                                s += str.Substring(1, 3) + " ";
                            }
                            continue;
                        }
                    }
                    break;
                }
                return s;
            }
        }

        public int StripColorInput { get; set; } = 0;

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

            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_LABEL, (byte)LabelInput));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_BTN_COLOR, (byte)ButtonColorInput));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM.BUTTON_STRIP_COLOR, (byte)StripColorInput));

            return list;
        }
    }
}
