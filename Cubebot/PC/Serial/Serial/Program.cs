using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
namespace Serial
{
     class Program
     {
          static void Main(string[] args)
          {
               SerialComm comm = new SerialComm();
               string comPort;
               foreach (string s in Serial.GetAvailablePorts())
               {
                    Console.WriteLine(s);
               }

               Console.WriteLine("Enter COM Port: ");
               comPort = "COM" + Console.ReadLine();
               if (comm.OpenPort(comPort))
               {

                   while (true)
                   {
                       Console.WriteLine("Commands \n  [0] Arms (Left: 0, Right: 1) \n  [1] Action (FaceCW: 0, FaceCCW: 1, CubeCW: 2, CubeCCW: 3) \n  [2] Rotate (D0: 0, D90: 1, D180: 2)");
                       string str = System.Console.ReadLine();
                       string[] arguments = str.Split(new char[] {' '});
                       byte[] message = new byte[arguments.Length];
                       for (int i = 0; i < message.Length; i++)
                       {
                           message[i] = byte.Parse(arguments[i]);
                       }
                       Console.WriteLine("Sending message...");
                       UInt16 time = 1000;
                       //comm.Send(SerialComm.SerialCommand.Debug, message, time);
                       //System.Threading.Thread.Sleep(1000);
                       comm.Send(SerialComm.SerialCommand.Robot, message, time);
                   }
                   comm.ClosePort();
               }
          }
     }
}
