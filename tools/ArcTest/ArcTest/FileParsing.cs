using Microsoft.Win32;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace ArcTest
{
    public static class FileParsing
    {
        const int NUM_LEDS = 15;
        //const int DEGREE = 6;
        const int DEGREE = 1;
        const int SEGMENTS = 360 / DEGREE;

        public static Color[] ParseFile(string path, bool experimental)
        {
            Color[] segments = null;

            if (File.Exists(path))
            {
                if (path.EndsWith(".txt"))
                {
                    try
                    {
                        string[] lines = File.ReadAllLines(path);
                        if (lines.Length != 360)
                        {
                            throw new Exception("Invalid file");
                        }

                        segments = new Color[NUM_LEDS * SEGMENTS];

                        for (int i = 0; i < lines.Length; i++)
                        {
                            string[] spoke = lines[i].Split(new char[] { ' ' },
                                StringSplitOptions.RemoveEmptyEntries);

                            if (spoke.Length != NUM_LEDS)
                            {
                                throw new Exception("Invalid file");
                            }

                            for (int j = 0; j < spoke.Length; j++)
                            {
                                string color = spoke[j];
                                if (color.Length != 8)
                                {
                                    throw new Exception("Invalid file");
                                }

                                byte a = Convert.ToByte(color.Substring(0, 2), 16);
                                byte r = Convert.ToByte(color.Substring(2, 2), 16);
                                byte g = Convert.ToByte(color.Substring(4, 2), 16);
                                byte b = Convert.ToByte(color.Substring(6, 2), 16);

                                segments[NUM_LEDS * i + j] = Color.FromArgb(a, r, g, b);
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new Exception($"Could not load file:\n\n{ex.Message}");
                    }
                }
                else if (path.EndsWith(".json"))
                {
                    try
                    {
                        string json = File.ReadAllText(path);
                        segments = JsonConvert.DeserializeObject<Color[]>(json);
                    }
                    catch (Exception ex)
                    {
                        throw new Exception($"Could not load file:\n\n{ex.Message}");
                    }
                }
                else if (path.EndsWith(".png") || 
                        path.EndsWith(".bmp")  || 
                        path.EndsWith(".jpg"))
                {
                    if (experimental)
                        segments = ImageParsing.ParseImage2(path);
                    else
                        segments = ImageParsing.ParseImage(path);
                }
            }

            return segments;
        }

        public static void SaveFile(string path, Color[] segments)
        {
            if (path.EndsWith(".txt"))
            {
                using (StreamWriter sw = new StreamWriter(path))
                {
                    for (int i = 0; i < segments.Length; i += NUM_LEDS)
                    {
                        for (int j = 0; j < NUM_LEDS; j++)
                        {
                            sw.Write(segments[i + j].ToString().Remove(0,1));
                            if (j < NUM_LEDS - 1)
                            {
                                sw.Write(" ");
                            }
                        }
                        sw.WriteLine();
                    }
                }
            }
            else if (path.EndsWith(".json"))
            {
                string json = JsonConvert.SerializeObject(segments, Formatting.Indented);
                File.WriteAllText(path, json);
            }
        }
    }
}
