using System;
using System.Reflection;

using Android.App;
using Android.Runtime;
using Xamarin.Android.NUnitLite;

namespace Xamarin.Android.DefaultInterfaceMethods_Tests
{
	[Instrumentation (Name="xamarin.android.dimtests.TestInstrumentation")]
	public class TestInstrumentation : TestSuiteInstrumentation {

		public TestInstrumentation (IntPtr handle, JniHandleOwnership transfer)
			: base (handle, transfer)
		{
		}

		protected override void AddTests ()
		{
			AddTest (Assembly.GetExecutingAssembly ());
		}
	}
}

