using System;

namespace Xamarin.Android.Prepare
{
	partial class Scenario_Standard
	{
		partial void AddRequiredOSSpecificSteps (bool beforeBundle)
		{
			if (beforeBundle) {
				Steps.Add (new Step_InstallAnt ());
			}
		}
	}
}
