using Android.App;
using Android.OS;
using Xamarin.Android.NUnitLite;
using System.Reflection;

namespace Xamarin.Android.DefaultInterfaceMethods_Tests
{
	[Activity (Label = "Xamarin.Android.DefaultInterfaceMethods-Tests", MainLauncher = true)]
	public class MainActivity : TestSuiteActivity
	{
		protected override void OnCreate (Bundle savedInstanceState)
		{
			AddTest (Assembly.GetExecutingAssembly ());

			base.OnCreate (savedInstanceState);
		}
	}
}
