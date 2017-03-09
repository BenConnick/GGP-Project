/*
 * Utility class for loading and parsing the beatmap file. 
 * Should totally be called "BeatmapParser" instead, but I
 * messed up oops
 */

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

namespace AudioGame
{
    // Audio parser
    public class AudioParser
    {

        ArrayList Items = new ArrayList();

        public string LoadMessage()
        {
            if (Items.Count > 0)
            {
                if ((string)Items[0] == "File is empty")
                {
                    return "file empty";
                } else
                {
                    return "File loaded";
                }
            }
            return "file not loaded";
        }

        public string GetItem(int index)
        {
            return (string)Items[index];
        }

        public AudioParser()
        {
            //string winDir = System.Environment.GetEnvironmentVariable("windir");

            //StreamReader reader = new StreamReader(winDir + "");
            StreamReader reader = new StreamReader("example.sm");
            try
            {
                do
                {
                    addListItem(reader.ReadLine());
                }
                while (reader.Peek() != -1);
                Debug.Log("Step Mania file loaded");
            }

            catch
            {
                Debug.Log("Could not load file");
                addListItem("File is empty");
            }

            finally
            {
                reader.Close();
            }

        }

        private void addListItem(string value)
        {
            Items.Add(value);
        }
    }   
}
