using KTaNE_Console.Core;
using KTaNE_Console.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KTaNE_Console.Modules
{
    public class ModuleFactory
    {
        private Serial serial;
        private IConsoleWriter CW;
        public ModuleFactory(Serial serial, IConsoleWriter ConsoleWriter)
        {
            this.serial = serial;
            this.CW = ConsoleWriter;

            this.serial.PacketReceived += Serial_PacketReceived;
        }

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {

        }
    }
}
