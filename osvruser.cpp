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

#include "osvruser.h"
#include <string>

OSVRUser::OSVRUser() {
  mGender = "male";
  setEye(OS, true, 32.5, 0.0, 0.0, 0.0, 0.0);
  setEye(OD, false, 32.5, 0.0, 0.0, 0.0, 0.0);
  //    setEye(&mLeft,"true",32.5,0.5,-1,80,2.5);
  //  setEye(&mRight,"false",32.5,-3.25,-.25,130,0.5);
  mAnthropometric.standingEyeHeight = 160;
  mAnthropometric.seatedEyeHeight = 106;
  mAnthropometric.eyeToNeck = 20.32;
}

void OSVRUser::setEye(eyeSide eyeBall, bool dominant, double pupilDistance,
                      double dSpherical, double dCylindrical, double dAxis,
                      double addNear) {

  eyeData *eD;

  if (eyeBall == OD)
    eD = &mRight;
  else
    eD = &mLeft;
  eD->dominant = dominant;
  eD->pupilDistance = pupilDistance;
  eD->correction.spherical = dSpherical;
  eD->correction.cylindrical = dCylindrical;
  eD->correction.axis = dAxis;
  eD->addNear = addNear;
}

string OSVRUser::gender() const { return mGender; }
void OSVRUser::setGender(const string &gender) { mGender = gender; }

double OSVRUser::eyeToNeck() const { return mAnthropometric.eyeToNeck; }

void OSVRUser::setEyeToNeck(double eyeToNeck) {
  mAnthropometric.eyeToNeck = eyeToNeck;
}

double OSVRUser::standingEyeHeight() const {
  return mAnthropometric.standingEyeHeight;
}

void OSVRUser::setStandingEyeHeight(double standingHeight) {
  mAnthropometric.standingEyeHeight = standingHeight;
}

double OSVRUser::seatedEyeHeight() const {
  return mAnthropometric.seatedEyeHeight;
}

void OSVRUser::setSeatedEyeHeight(double seatedHeight) {
  mAnthropometric.seatedEyeHeight = seatedHeight;
}

bool OSVRUser::dominant(eyeSide eyeBall) const {
  if (eyeBall == OS)
    return mLeft.dominant;
  else
    return mRight.dominant;
}
void OSVRUser::setDominant(eyeSide eyeBall) {
  switch (eyeBall) {
  case OS: {
    mLeft.dominant = true;
    mRight.dominant = false;
  } break;
  case OD: {
    mLeft.dominant = false;
    mRight.dominant = true;
  } break;
  }
}

double OSVRUser::pupilDistance(eyeSide eyeBall) const {
  switch (eyeBall) {
  case OS:
    return mLeft.pupilDistance;
    break;
  case OD:
  default:
    return mRight.pupilDistance;
  }
}

void OSVRUser::setPupilDistance(eyeSide eyeBall, double ipd) {
  switch (eyeBall) {
  case OS:
    mLeft.pupilDistance = ipd;
    break;
  case OD:
    mRight.pupilDistance = ipd;
  }
}

double OSVRUser::spherical(eyeSide eyeBall) const {
  switch (eyeBall) {
  case OS:
    return mLeft.correction.spherical;
    break;
  case OD:
  default:
    return mRight.correction.spherical;
  }
}

void OSVRUser::setSpherical(eyeSide eyeBall, double spherical) {
  switch (eyeBall) {
  case OS:
    mLeft.correction.spherical = spherical;
    break;
  case OD:
    mRight.correction.spherical = spherical;
  }
}

double OSVRUser::cylindrical(eyeSide eyeBall) const {
  switch (eyeBall) {
  case OS:
    return mLeft.correction.cylindrical;
    break;
  case OD:
  default:
    return mRight.correction.cylindrical;
  }
}

void OSVRUser::setCylindrical(eyeSide eyeBall, double cylindrical) {
  switch (eyeBall) {
  case OS:
    mLeft.correction.cylindrical = cylindrical;
    break;
  case OD:
    mRight.correction.cylindrical = cylindrical;
  }
}

double OSVRUser::axis(eyeSide eyeBall) const {
  switch (eyeBall) {
  case OS:
    return mLeft.correction.axis;
    break;
  case OD:
  default:
    return mRight.correction.axis;
  }
}

void OSVRUser::setAxis(eyeSide eyeBall, double axis) {
  switch (eyeBall) {
  case OS:
    mLeft.correction.axis = axis;
    break;
  case OD:
    mRight.correction.axis = axis;
  }
}

double OSVRUser::addNear(eyeSide eyeBall) const {
  switch (eyeBall) {
  case OS:
    return mLeft.addNear;
    break;
  case OD:
  default:
    return mRight.addNear;
  }
}

void OSVRUser::setAddNear(eyeSide eyeBall, double addNear) {
  switch (eyeBall) {
  case OS:
    mLeft.addNear = addNear;
    break;
  case OD:
    mRight.addNear = addNear;
  }
}

void OSVRUser::readPersonal(const Json::Value qq) {
  string gender = qq["gender"].asString();

  if (gender == "Male")
    mGender = "Male";
  else
    mGender = "Female";

  Json::Value anthro = qq["anthropometric"];
  mAnthropometric.standingEyeHeight = anthro["standingEyeHeight"].asDouble();
  mAnthropometric.seatedEyeHeight = anthro["seatedEyeHeight"].asDouble();
  mAnthropometric.eyeToNeck = anthro["eyeToNeck"].asDouble();

  Json::Value eye;
  eye = qq["eyes"];
  readEye(&mLeft, eye["left"]);
  readEye(&mRight, eye["right"]);
}

void OSVRUser::readEye(eyeData *eD, const Json::Value qq) {

  eD->pupilDistance = qq["pupilDistance"].asDouble();
  eD->dominant = qq["dominant"].asBool();

  Json::Value correction;
  correction = qq["correction"];

  Json::Value distance = correction["distance"];
  eD->correction.spherical = distance["spherical"].asDouble();
  eD->correction.cylindrical = distance["cylindrical"].asDouble();
  eD->correction.axis = distance["axis"].asDouble();

  Json::Value addNear = correction["addNear"];
  eD->addNear = addNear["spherical"].asDouble();
}

void OSVRUser::read(const Json::Value json) {
  readPersonal(json["personalSettings"]);
}

void OSVRUser::write(Json::Value &json) const {
  Json::Value oo;
  writePersonal(oo);
  json["personalSettings"] = oo;
}

void OSVRUser::writePersonal(Json::Value &personalSettingsJson) const {
  Json::Value anthropometricJson;
  anthropometricJson["eyeToNeck"] = mAnthropometric.eyeToNeck;
  anthropometricJson["seatedEyeHeight"] = mAnthropometric.seatedEyeHeight;
  anthropometricJson["standingEyeHeight"] = mAnthropometric.standingEyeHeight;
  personalSettingsJson["anthropometric"] = anthropometricJson;

  Json::Value eyesJson, od, os;
  writeEye(mLeft, os);
  eyesJson["left"] = os;
  writeEye(mRight, od);
  eyesJson["right"] = od;
  personalSettingsJson["gender"] = mGender;
  personalSettingsJson["eyes"] = eyesJson;
}

void OSVRUser::writeEye(eyeData e, Json::Value &eyeJson) const {
  eyeJson["dominant"] = e.dominant;
  eyeJson["pupilDistance"] = e.pupilDistance;

  Json::Value distance;
  distance["axis"] = e.correction.axis;
  distance["cylindrical"] = e.correction.cylindrical;
  distance["spherical"] = e.correction.spherical;

  Json::Value correction, near;
  near["spherical"] = e.addNear;
  correction["addNear"] = near;
  correction["distance"] = distance;
  eyeJson["correction"] = correction;
}
