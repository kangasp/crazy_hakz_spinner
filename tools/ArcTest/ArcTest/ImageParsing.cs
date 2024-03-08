using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace ArcTest
{
    public static class ImageParsing
    {
        const int NUM_LEDS = 15;
        //const int DEGREE = 6;
        const int DEGREE = 1;
        const int SEGMENTS = 360 / DEGREE;

        private static System.Drawing.Bitmap GetImage(string imagePath)
        {
            // Load the PNG image
            System.Drawing.Bitmap image = new System.Drawing.Bitmap(imagePath);

            // Convert the image to an RGBA format (if not already)
            System.Drawing.Bitmap rgbaImage = new System.Drawing.Bitmap(image.Width, image.Height, 
                System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(rgbaImage))
            {
                g.DrawImage(image, new System.Drawing.Rectangle(0, 0, image.Width, image.Height));
            }

            if (image.Width != image.Height)
            {
                MessageBox.Show("Image is not square. Image will be cropped,", "Image Error",
                    MessageBoxButton.OK, MessageBoxImage.Question);
            }

            return rgbaImage;
        }

        public static Color[] ParseImage(string imagePath)
        {
            System.Drawing.Bitmap image = GetImage(imagePath);

            //Get the smaller bounds of the image to use for the radius
            int imageBounds = Math.Min(image.Width, image.Height);

            // Initialize the Color array
            Color[] colorArray = new Color[NUM_LEDS * SEGMENTS];

            int centerX = image.Width / 2;
            int centerY = image.Height / 2;
            Point center = new Point(centerX, centerY);

            for (int degree = 0; degree < SEGMENTS; degree++)
            {
                for (int radius = 0; radius < NUM_LEDS; radius++)
                {
                    int interpolatedRadius = (int)Math.Round(radius * (((double)imageBounds / 2) / 15));

                    Point point = Polar.PolarToCartesian(degree, interpolatedRadius, center);

                    System.Drawing.Color color = image.GetPixel((int)point.X, (int)point.Y);
                    colorArray[degree * NUM_LEDS + radius] = Color.FromArgb(color.A, color.R, color.G, color.B);
                }
            }

            return colorArray;
        }

        public static Color[] ParseImage2(string imagePath)
        {
            System.Drawing.Bitmap image = GetImage(imagePath);

            //Get the smaller bounds of the image to use for the radius
            int imageBounds = Math.Min(image.Width, image.Height);

            // Initialize the Color array
            List<Color>[,] colorArray = new List<Color>[SEGMENTS, NUM_LEDS];

            int centerX = image.Width / 2;
            int centerY = image.Height / 2;
            Point center = new Point(centerX, centerY);
            Point topLeft = new Point(centerX - (imageBounds / 2), centerY - (imageBounds / 2));

            for (int y = (int)topLeft.Y; y < imageBounds; y++)
            {
                for (int x = (int)topLeft.X; x < imageBounds; x++)
                {
                    Polar polar = Polar.CartesianToPolar(new Point(x, y),
                        new Rect(new Size(imageBounds, imageBounds)));

                    if (polar.R >= imageBounds / 2)
                        continue;

                    int degree = (int)polar.A;
                    int radius = (int)(polar.R / (((double)imageBounds / 2) / NUM_LEDS));

                    if (degree >= SEGMENTS ||
                        radius >= NUM_LEDS)
                        throw new Exception("Somehow out of range");

                    if (colorArray[degree, radius] == null)
                        colorArray[degree, radius] = new List<Color>();

                    System.Drawing.Color color = image.GetPixel(x, y);

                    colorArray[degree, radius].Add(Color.FromArgb(color.A, color.R, color.G, color.B));
                }
            }

            Color[] colors = new Color[SEGMENTS * NUM_LEDS];

            for (int i = 0; i < SEGMENTS; i++)
            {
                for (int j = 0; j < NUM_LEDS; j++)
                {
                    if (colorArray[i, j] == null)
                        continue;

                    List<Color> listColors = colorArray[i, j];

                    byte avgA = (byte)listColors.Average(c => c.A);
                    byte avgR = (byte)listColors.Average(c => c.R);
                    byte avgG = (byte)listColors.Average(c => c.G);
                    byte avgB = (byte)listColors.Average(c => c.B);

                    colors[i * NUM_LEDS + j] = Color.FromArgb(avgA, avgR, avgG, avgB);
                }
            }

            return colors;
        }
    }
}
