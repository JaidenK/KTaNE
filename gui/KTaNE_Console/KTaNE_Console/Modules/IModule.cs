namespace KTaNE_Console.Modules
{
    public enum SolvedState
    {
        SOLVED,
        UNSOLVED,
    }

    public interface IModule
    {
        byte BusAddress { get; }
        string ModuleName { get; }
        string ModelID { get; }
        SolvedState State { get; }
        byte Status { get; set; }
        EEPROM Eeprom { get; set; }
    }
}
