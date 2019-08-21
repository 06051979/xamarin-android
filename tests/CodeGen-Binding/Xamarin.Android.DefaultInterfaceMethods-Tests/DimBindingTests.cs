using System;
using Com.Xamarin.Test;
using Java.Lang;
using NUnit.Framework;

namespace Xamarin.Android.DefaultInterfaceMethodsTests
{
	[TestFixture]
	public class DimTest
	{
		[Test]
		public void TestDefaultInterfaceMethods ()
		{
			var empty = new EmptyOverrideClass ();
			var iface = empty as IDefaultMethodsInterface;

			Assert.AreEqual (0, iface.Foo ());
			Assert.AreEqual (2, iface.Bar);
			Assert.DoesNotThrow (() => iface.Bar = 5);

			Assert.AreEqual (0, iface.InvokeFoo ());

			Assert.Throws<UnsupportedOperationException> (() => iface.ToImplement ());
		}

		[Test]
		public void TestOverriddenDefaultInterfaceMethods ()
		{
			var over = new ImplementedOverrideClass ();
			var iface = over as IDefaultMethodsInterface;

			Assert.AreEqual (6, over.Foo ());
			Assert.AreEqual (100, over.Bar);
			Assert.DoesNotThrow (() => over.Bar = 5);

			Assert.AreEqual (6, iface.InvokeFoo ());
		}
	}
}
