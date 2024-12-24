using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KTaNE_Console.Modules
{
    public interface IModulePanel
    {
        List<byte[]> BuildSetEEPROMPackets();
    }
}
