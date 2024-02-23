using Microsoft.Win32;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Permissions;
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

namespace ArcTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int NUM_LEDS = 15;
        const int DEGREE = 6;
        //const int DEGREE = 1;
        const int SEGMENTS = 360 / DEGREE;

        Color[] segments = new Color[NUM_LEDS * SEGMENTS];

        Color currentColor = Colors.Red;
        Color colorZero = Color.FromArgb(0,0,0,0);

        public MainWindow()
        {
            InitializeComponent();

            int counter = 0;

            for (int i = 0; i < SEGMENTS; i++)
            {
                for (int j = 0; j < NUM_LEDS; j++)
                {
                    ArcII arc = new ArcII();
                    arc.Center = new Point(0,0);
                    arc.OverrideCenter = true;
                    arc.StartAngle = i * DEGREE;
                    arc.SweepAngle = DEGREE;
                    arc.Radius = 10 + (j * NUM_LEDS) + 5;
                    arc.Stroke = new SolidColorBrush(Colors.Gray);
                    arc.StrokeThickness = 10;
                    arc.MouseDown += Arc_MouseDown;
                    arc.MouseEnter += Arc_MouseEnter;

                    arc.Tag = counter++;

                    grid.Children.Add(arc);
                }
            }
        }

        private void Arc_MouseEnter(object sender, MouseEventArgs e)
        {
            ArcII arc = (ArcII)sender;
            int count = (int)arc.Tag;

            if (e.LeftButton == MouseButtonState.Pressed)
            {
                segments[count] = currentColor;
                arc.Stroke = new SolidColorBrush(segments[count]);
            }
            else if (e.RightButton == MouseButtonState.Pressed)
            {
                segments[count] = colorZero;
                arc.Stroke = new SolidColorBrush(Colors.Gray);
            }
        }

        private void Arc_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                ArcII arc = (ArcII)sender;
                int count = (int)arc.Tag;
                if (segments[count] == currentColor)
                {
                    segments[count] = colorZero;
                    arc.Stroke = new SolidColorBrush(Colors.Gray);
                }
                else
                {
                    segments[count] = currentColor;
                    arc.Stroke = new SolidColorBrush(segments[count]);
                }
            }
        }

        private void Button_Click_Copy(object sender, RoutedEventArgs e)
        {
            StringBuilder sb = new StringBuilder();

            sb.AppendLine("{");

            for (int i = 0; i < SEGMENTS; i++)
            {
                sb.Append("    { ");
                for (int j = 0; j < NUM_LEDS; j++)
                {
                    //sb.Append($"{{ 0xE3, 0x{segments[15*i+j].R.ToString("X2")}, 0x{segments[15*i + j].B.ToString("X2")}, 0x{segments[15*i + j].G.ToString("X2")} }}, ");
                    Color c = segments[NUM_LEDS * i + j];
                    sb.Append($"0x00{c.R:X2}{c.G:X2}{c.B:X2}, ");
                }

                sb.Length -= 2;

                sb.AppendLine(" },");
            }

            sb.Length -= 3;
            sb.AppendLine();

            sb.AppendLine("}; ");

            Clipboard.SetText(sb.ToString());
        }
        private void Button_Click_Save(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "JSON Files|*.json";
            if (saveFileDialog.ShowDialog() ?? false)
            {
                string json = JsonConvert.SerializeObject(segments, Formatting.Indented);

                File.WriteAllText(saveFileDialog.FileName, json);
            }
        }

        private void RefreshArcs()
        {
            foreach (var arc in grid.Children.OfType<ArcII>())
            {
                int index = (int)arc.Tag;
                if (segments[index] == colorZero)
                {
                    arc.Stroke = new SolidColorBrush(Colors.Gray);
                }
                else
                {
                    arc.Stroke = new SolidColorBrush(segments[index]);
                }
            }
        }

        private void Button_Click_Load(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "JSON Files|*.json";
            openFileDialog.CheckFileExists = true;
            openFileDialog.CheckPathExists = true;

            if (openFileDialog.ShowDialog() ?? false)
            {
                if (File.Exists(openFileDialog.FileName))
                {
                    string json = File.ReadAllText(openFileDialog.FileName);
                    segments = JsonConvert.DeserializeObject<Color[]>(json);

                    RefreshArcs();
                }
            }
        }

        private void Button_Click_Clear(object sender, RoutedEventArgs e)
        {
            segments = new Color[NUM_LEDS * SEGMENTS];
            RefreshArcs();
        }

        private void Button_Click_Set(object sender, RoutedEventArgs e)
        {
            string input = txtColor.Text
                .Replace("#", "")
                .Replace("0x", "")
                .Replace(" ", "");

            if (input.Length == 8)
            {
                input = input.Remove(0, 2);
            }

            if (input.Length == 6)
            {
                try
                {
                    byte r = Convert.ToByte(input.Substring(0, 2), 16);
                    byte g = Convert.ToByte(input.Substring(2, 2), 16);
                    byte b = Convert.ToByte(input.Substring(4, 2), 16);

                    currentColor = Color.FromRgb(r, g, b);
                    rectCur.Fill = new SolidColorBrush(currentColor);
                }
                catch { }
            }
        }

        private void Button_Click_Color(object sender, RoutedEventArgs e)
        {
            Button button = (Button)sender;
            currentColor = ((SolidColorBrush)button.Background).Color;
            rectCur.Fill = new SolidColorBrush(currentColor);
        }
    }
}
