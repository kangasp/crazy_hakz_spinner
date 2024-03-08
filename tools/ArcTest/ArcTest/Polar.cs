using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace ArcTest
{
    public class Polar
    {
        public double A { get; set; }
        public double R { get; set; }
        public double Q { get; set; }
        private Polar(double angle, double radius, double q)
        {
            A = angle;
            R = radius;
            Q = q;
        }

        public static Polar CartesianToPolar(Point point, Rect rect)
        {
            point = new Point(point.X - rect.Width / 2, point.Y - rect.Height / 2);

            double r = Math.Sqrt((point.X * point.X) + (point.Y * point.Y));
            double q = Math.Atan2(point.Y, point.X); // Angle in radians
            double deg = q * 180 / Math.PI; // Convert angle to degrees
            if (deg < 0)
                deg += 360;

            return new Polar(deg, r, q);
        }


        /// <summary>
        /// Given the center of a circle and its radius, along with the angle 
        /// corresponding to the point, find the coordinates.  In other words, 
        /// convert from polar to rectangular coordinates.
        /// </summary>
        /// <param name="angle"></param>
        /// <param name="radius"></param>
        /// <param name="center"></param>
        /// <returns></returns>
        public static Point PolarToCartesian(double angle, double radius, Point center)
        {
            return new Point((center.X + (radius * Math.Cos(DegreesToRadian(angle)))), (center.Y + (radius * Math.Sin(DegreesToRadian(angle)))));
        }


        /// <summary>
        /// Given a center point and radius, find the top left point for a rectangle and its size.
        /// </summary>
        /// <param name="centerPoint"></param>
        /// <param name="radius"></param>
        /// <returns></returns>
        public static Rect RectFromCenterPoint(Point centerPoint, int radius)
        {
            Point p = new Point(centerPoint.X - radius, centerPoint.Y - radius);
            return new Rect(p, new Size(radius * 2, radius * 2));
        }

        /// <summary>
        /// Finds the center point of a Rect
        /// </summary>
        /// <param name="rect"></param>
        /// <returns></returns>
        public static Point CenterPoint(Rect rect)
        {
            return new Point(rect.Width / 2, rect.Height / 2);
        }

        /// <summary>
        /// Returns a radius value equal to the smallest side.
        /// </summary>
        /// <param name="rect"></param>
        /// <returns></returns>
        public static double Radius(Rect rect)
        {
            double dbl = Math.Min(rect.Width, rect.Height);
            return dbl / 2;
        }


        /// <summary>
        /// Since Windows Forms consider an Angle of Zero to be at the 3:00 position and an Angle of 90
        /// to be at the 12:00 position, it is sometimes difficult to visualize where 
        /// 
        /// </summary>
        /// <param name="Angle"></param>
        /// <param name="Offset"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public static float ReversePolarDirection(float Angle, int Offset)
        {
            return ((360 - Angle) + Offset) % 360;
        }

        /// <summary>
        /// Circumference: C = 2*Pi*r = Pi*d; r=Radius, d=Diameter
        /// </summary>
        /// <param name="Diameter"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public static double CircumferenceD(double Diameter)
        {
            return Diameter * Math.PI;
        }
        /// <summary>
        /// Circumference: C = 2*Pi*r = Pi*d; r=Radius, d=Diameter
        /// </summary>
        /// <param name="Radius"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public static double CircumferenceR(double Radius)
        {
            return Radius * Math.PI;
        }
        public static double ScaleWithParam(double Input, double InputMin, double InputMax, double ScaledMin, double ScaledMax)
        {
            //Out = (((ScMax-ScMin)/(InMax-InMin))*Input)+(ScMin-(InMin*((ScMax-ScMin)/(InMax-InMin))
            return (((ScaledMax - ScaledMin) / (InputMax - InputMin)) * Input) + (ScaledMin - (InputMin * ((ScaledMax - ScaledMin) / (InputMax - InputMin))));

        }
        public static double DegreesToRadian(double degrees)
        {
            //Return 2 * Math.PI * degrees / 360.0
            return degrees * (Math.PI / 180);
        }
        private static double RadianToDegrees(double radian)
        {
            return radian * 180 / Math.PI;
        }

        public static double ArcLength(double radius, double radian)
        {
            return radius * radian;
        }
    }
}
