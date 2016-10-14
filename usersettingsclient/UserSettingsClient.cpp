/*
 * Copyright 2016 OSVR and contributors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// Internal Includes
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include <osvr/ClientKit/InterfaceStateC.h>

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

int main() {
  osvr::clientkit::ClientContext context("com.osvr.UserSettingsClient");

  // This is just one of the paths: specifically, the Hydra's left
  // controller's analog trigger. More are in the docs and/or listed on
  // startup
  osvr::clientkit::Interface IPD = context.getInterface("/me/IPD");

  osvr::clientkit::Interface StandingHeight =
      context.getInterface("/me/StandingHeight");

  osvr::clientkit::Interface SeatedHeight =
      context.getInterface("/me/SeatedHeight");

  // Pretend that this is your application's mainloop.
  for (int i = 0; i < 100000000; ++i) {
    context.update();

    if ((i % 1000) == 0 || 1) {
      OSVR_AnalogState ipdState, standingState, seatedState;
      OSVR_TimeValue timestamp;
      OSVR_ReturnCode ret =
          osvrGetAnalogState(IPD.get(), &timestamp, &ipdState);
      osvrGetAnalogState(StandingHeight.get(), &timestamp, &standingState);
      osvrGetAnalogState(SeatedHeight.get(), &timestamp, &seatedState);
      if (OSVR_RETURN_SUCCESS != ret) {
        std::cout << "state read failure" << std::endl;
      } else {
        //				std::cout << timestamp.seconds << "." <<
        //timestamp.microseconds;
        std::cout << "IPD: " << ipdState << " Standing: " << standingState
                  << " Seated: " << seatedState << std::endl;
      }
    }
  }

  std::cout << "Library shut down, exiting." << std::endl;
}
