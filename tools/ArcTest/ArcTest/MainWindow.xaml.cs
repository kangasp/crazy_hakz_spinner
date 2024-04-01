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
using System.Windows.Threading;

namespace ArcTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int NUM_LEDS = 15;
        //const int DEGREE = 6;
        const int DEGREE = 1;
        const int SEGMENTS = 360 / DEGREE;

        Color[] segments = new Color[NUM_LEDS * SEGMENTS];

        Color currentColor = Colors.Red;
        Color colorZero = Color.FromArgb(0,0,0,0);

        DispatcherTimer timer = new DispatcherTimer();

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

            timer.Interval = TimeSpan.FromSeconds(10);
            timer.Tick += Timer_Tick;
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            timer.Stop();

            string json = JsonConvert.SerializeObject(segments, Formatting.Indented);

            try
            {
                File.WriteAllText("temp.json", json);
            }
            catch { }
        }

        private void StartTimer()
        {
            if (!timer.IsEnabled)
            {
                timer.Interval = TimeSpan.FromSeconds(10);
                timer.Start();
            }
        }

        private void DoColor(ArcII arc, int count, bool set)
        {
            if (set)
            {
                segments[count] = currentColor;
                arc.Stroke = new SolidColorBrush(currentColor);
            }
            else
            {
                segments[count] = colorZero;
                arc.Stroke = new SolidColorBrush(Colors.Gray);
            }

            StartTimer();
        }

        private void Arc_MouseEnter(object sender, MouseEventArgs e)
        {
            ArcII arc = (ArcII)sender;
            int count = (int)arc.Tag;

            if (e.LeftButton == MouseButtonState.Pressed)
            {
                DoColor(arc, count, true);
            }
            else if (e.RightButton == MouseButtonState.Pressed)
            {
                DoColor(arc, count, false);
            }
        }

        private void Arc_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ArcII arc = (ArcII)sender;
            int count = (int)arc.Tag;
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                //DoColor(arc, count, segments[count] != currentColor);
                DoColor(arc, count, true);
            }
            else if (e.RightButton == MouseButtonState.Pressed)
            {
                DoColor(arc, count, false);
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
                    Color c = segments[NUM_LEDS * i + j];
                    sb.Append($"{{ 0x{c.R:X2}, 0x{c.B:X2}, 0x{c.G:X2} }}, ");
                    //sb.Append($"0x00{c.R:X2}{c.G:X2}{c.B:X2}, ");
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
            saveFileDialog.Filter = "Binary Files|*.bin|JSON Files|*.json|Text Files|*.txt";
            if (saveFileDialog.ShowDialog() ?? false)
            {
                try
                {
                    FileParsing.SaveFile(saveFileDialog.FileName, segments);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
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
            openFileDialog.Filter = "All Files|*.*|JSON Files|*.json|Text Files|*.txt|Image Files|*.jpg;*.png;*.bmp";
            openFileDialog.CheckFileExists = true;
            openFileDialog.CheckPathExists = true;

            if (openFileDialog.ShowDialog() ?? false)
            {
                try
                {
                    segments = FileParsing.ParseFile(openFileDialog.FileName, chkExperimental.IsChecked ?? false);

                    RefreshArcs();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
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
            txtColor.Text = currentColor.ToString();
        }

        private void grid_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (e.Delta >= 1)
            {
                slrZoom.Value += 0.1;
            }
            else
            {
                slrZoom.Value -= 0.1;
            }
        }
    }
}
