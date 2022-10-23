using KTaNE_Console.Core;
using System.Collections.Generic;

namespace KTaNE_Console.Model
{
    public class BombConfig : ObservableObject
    {
        public string SerialNo { get; set; } = "";
        public int nBatteries { get; set; } = 0;
        public List<string> Indicators { get; set; } = new List<string>();
        public List<string> Ports { get; set; } = new List<string>();
        public int TimeLimit_s { get; set; } = 0;
    }
}
