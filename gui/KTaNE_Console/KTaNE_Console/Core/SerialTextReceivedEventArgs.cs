using System;

namespace KTaNE_Console.Core
{
    public class SerialTextReceivedEventArgs : EventArgs
    {
        public string text;

        public SerialTextReceivedEventArgs(string text)
        {
            this.text = text;
        }
    }
}