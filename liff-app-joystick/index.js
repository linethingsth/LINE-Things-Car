// User service UUID: Change this to your generated service UUID
const USER_SERVICE_UUID = '36752cf3-739c-4625-aea1-4d47a70ad656'; // LED, Button
// User service characteristics

const WRITE_CHARACTERISTIC_UUID = "E9062E71-9E62-4BC6-B0D3-35CDCD9B027B" // LED
const NOTIFY_CHARACTERISTIC_UUID = "62FBD229-6EDD-4D1A-B554-5C4E1BB29169" // button

const DIRECTION_CHARACTERISTIC_UUID = 'fbfeb748-be33-4896-9f0f-8cd5a86ea961';
const SPEED_CHARACTERISTIC_UUID = 'ff4fb94b-98b5-405f-9d4d-1d97b2b12263';
const LEFT_SENSOR_CHARACTERISTIC_UUID = '5380b1d9-4d37-47b1-ac48-ead2df7b7135';
const RIGHT_SENSOR_CHARACTERISTIC_UUID = '6c626864-9ae0-4054-822c-1130d8f6f45c';

// PSDI Service UUID: Fixed value for Developer Trial
const PSDI_SERVICE_UUID = 'E625601E-9E55-4597-A598-76018A0D293D'; // Device ID
const PSDI_CHARACTERISTIC_UUID = '26E2B12B-85F0-4F3F-9FDD-91D114270E6E';

// UI settings
let ledState = false; // true: LED on, false: LED off
let clickCount = 0;

// -------------- //
// On window load //
// -------------- //

window.onload = () => {
  initializeApp();
};

// ----------------- //
// Handler functions //
// ----------------- //

// ------------ //
// UI functions //
// ------------ //

function uiCountPressButton() {
  clickCount++;

  const el = document.getElementById("click-count");
  el.innerText = clickCount;
}

function uiToggleStateButton(pressed) {
  const el = document.getElementById("btn-state");

  if (pressed) {
    el.classList.add("pressed");
    el.innerText = "Pressed";
  } else {
    el.classList.remove("pressed");
    el.innerText = "Released";
  }
}

function uiToggleDeviceConnected(connected) {
  const elStatus = document.getElementById("status");
  // const elControls = document.getElementById("controls");

  elStatus.classList.remove("error");

  if (connected) {
    // Hide loading animation
    uiToggleLoadingAnimation(false);
    // Show status connected
    elStatus.classList.remove("inactive");
    elStatus.classList.add("success");
    elStatus.innerText = "Device connected";
    // Show controls
    // elControls.classList.remove("hidden");
  } else {
    // Show loading animation
    uiToggleLoadingAnimation(true);
    // Show status disconnected
    elStatus.classList.remove("success");
    elStatus.classList.add("inactive");
    elStatus.innerText = "Device disconnected";
    // Hide controls
    // elControls.classList.add("hidden");
  }
}

function uiToggleLoadingAnimation(isLoading) {
  const elLoading = document.getElementById("loading-animation");

  if (isLoading) {
    // Show loading animation
    elLoading.classList.remove("hidden");
  } else {
    // Hide loading animation
    elLoading.classList.add("hidden");
  }
}

function uiStatusError(message, showLoadingAnimation) {
  uiToggleLoadingAnimation(showLoadingAnimation);

  const elStatus = document.getElementById("status");
  // const elControls = document.getElementById("controls");

  // Show status error
  elStatus.classList.remove("success");
  elStatus.classList.remove("inactive");
  elStatus.classList.add("error");
  elStatus.innerText = message;

  // Hide controls
  // elControls.classList.add("hidden");
}

function makeErrorMsg(source, errorObj) {
  return "Error\n" + "Source: " + source + "\n" + errorObj.code + "\n" + errorObj.message;
}

// -------------- //
// LIFF functions //
// -------------- //

function initializeApp() {
  liff.init(() => initializeLiff(), error => uiStatusError(makeErrorMsg("initializeApp", error), false));
}

function initializeLiff() {
  liff.initPlugins(['bluetooth']).then(() => {
    liffCheckAvailablityAndDo(() => liffRequestDevice());
  }).catch(error => {
    uiStatusError(makeErrorMsg("initializeLiff", error), false);
  });
}

function liffCheckAvailablityAndDo(callbackIfAvailable) {
  // Check Bluetooth availability
  liff.bluetooth.getAvailability().then(isAvailable => {
    if (isAvailable) {
      uiToggleDeviceConnected(false);
      callbackIfAvailable();
    } else {
      uiStatusError("Bluetooth not available", true);
      setTimeout(() => liffCheckAvailablityAndDo(callbackIfAvailable), 10000);
    }
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffCheckAvailablityAndDo", error), false);
  });;
}

function liffRequestDevice() {
  liff.bluetooth.requestDevice().then(device => {
    liffConnectToDevice(device);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffRequestDevice", error), false);
  });
}

function liffConnectToDevice(device) {
  device.gatt.connect().then(() => {
    // document.getElementById("device-name").innerText = device.name;
    // document.getElementById("device-id").innerText = device.id;

    // Show status connected
    uiToggleDeviceConnected(true);

    // Get service
    device.gatt.getPrimaryService(USER_SERVICE_UUID).then(service => {
      liffGetUserService(service);
    }).catch(error => {
      uiStatusError(makeErrorMsg("liffConnectToDevice: USER_SERVICE_UUID", error), false);
    });
    device.gatt.getPrimaryService(PSDI_SERVICE_UUID).then(service => {
      liffGetPSDIService(service);
    }).catch(error => {
      uiStatusError(makeErrorMsg("liffConnectToDevice: USER_SERVICE_UUID", error), false);
    });

    // Device disconnect callback
    const disconnectCallback = () => {
      // Show status disconnected
      uiToggleDeviceConnected(false);

      // Remove disconnect callback
      device.removeEventListener('gattserverdisconnected', disconnectCallback);

      // Reset LED state
      ledState = false;
      // Reset UI elements
      // uiToggleLedButton(false);
      // uiToggleStateButton(false);

      // Try to reconnect
      initializeLiff();
    };

    device.addEventListener('gattserverdisconnected', disconnectCallback);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffConnectToDevice: connect", error), false);
  });
}

function liffGetUserService(service) {
  // Notify Characteristic
  // service.getCharacteristic(NOTIFY_CHARACTERISTIC_UUID).then(characteristic => {
  //   liffGetNotifyCharacteristic(characteristic);
  // }).catch(error => {
  //   uiStatusError(makeErrorMsg(error), false);
  // });

  // LEFT Sensor
  service.getCharacteristic(LEFT_SENSOR_CHARACTERISTIC_UUID).then(characteristic => {
    liffGetLeftSensorCharacteristic(characteristic);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetUserService: LEFT_SENSOR_CHARACTERISTIC_UUID", error), false);
  });

  // RIGHT Sensor
  service.getCharacteristic(RIGHT_SENSOR_CHARACTERISTIC_UUID).then(characteristic => {
    liffGetRightSensorCharacteristic(characteristic);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetUserService: RIGHT_SENSOR_CHARACTERISTIC_UUID", error), false);
  });


  // Direction
  service.getCharacteristic(DIRECTION_CHARACTERISTIC_UUID).then(characteristic => {
    window.directionCharacteristic = characteristic;

    // Direction 0
    liffSendDirectionState(0);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetUserService: DIRECTION_CHARACTERISTIC_UUID", error), false);
  });


  // Speed
  service.getCharacteristic(SPEED_CHARACTERISTIC_UUID).then(characteristic => {
    window.speedCharacteristic = characteristic;

    // Speed 0
    liffSendSpeedState(0);
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetUserService: SPEED_CHARACTERISTIC_UUID", error), false);
  });
}

function liffGetPSDIService(service) {
  // Get PSDI value
  service.getCharacteristic(PSDI_CHARACTERISTIC_UUID).then(characteristic => {
    return characteristic.readValue();
  }).then(value => {
    // Byte array to hex string
    const psdi = new Uint8Array(value.buffer)
      .reduce((output, byte) => output + ("0" + byte.toString(16)).slice(-2), "");
    // document.getElementById("device-psdi").innerText = psdi;
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetPSDIService: PSDI_CHARACTERISTIC_UUID", error), false);
  });
}

function liffGetNotifyCharacteristic(characteristic) {
  // Add notification hook for left sensor
  // (Get notified when left sensor changes)
  characteristic.startNotifications().then(() => {
    characteristic.addEventListener('characteristicvaluechanged', e => {
      const val = (new Uint8Array(e.target.value.buffer))[0];
      console.log("Notify Characteristic", val)
    });
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetNotifyCharacteristic", error), false);
  });
}

function liffGetLeftSensorCharacteristic(characteristic) {
  // Add notification hook for left sensor
  // (Get notified when left sensor changes)
  characteristic.startNotifications().then(() => {
    characteristic.addEventListener('characteristicvaluechanged', e => {
      const val = (new Uint8Array(e.target.value.buffer))[0];
      if (val > 0) {
        setSensor(true, val)
      } else {
        setSensor(true, 0)
      }
    });
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetLeftSensorCharacteristic", error), false);
  });
}

function liffGetRightSensorCharacteristic(characteristic) {
  // Add notification hook for right sensor
  // (Get notified when right sensor changes)
  characteristic.startNotifications().then(() => {
    characteristic.addEventListener('characteristicvaluechanged', e => {
      const val = (new Uint8Array(e.target.value.buffer))[0];
      if (val > 0) {
        setSensor(false, val)
      } else {
        setSensor(false, 0)
      }
    });
  }).catch(error => {
    uiStatusError(makeErrorMsg("liffGetRightSensorCharacteristic", error), false);
  });
}

function setSensor(left, val) {
  if (left) {
    symbol = ".wifi-symbol-1"
  } else {
    symbol = ".wifi-symbol-2"
  }

  fourth = document.querySelector(symbol + " .wifi-circle.fourth");
  third = document.querySelector(symbol + " .wifi-circle.third");
  second = document.querySelector(symbol + " .wifi-circle.second");
  first = document.querySelector(symbol + " .wifi-circle.first");

  setSensorActive(fourth, false)
  setSensorActive(third, false)
  setSensorActive(second, false)
  setSensorActive(first, false)

  if (val > 0) {
    setSensorActive(fourth, true)

    if (val > 25) {
      setSensorActive(third, true)
    }

    if (val > 50) {
      setSensorActive(second, true)
    }

    if (val > 75) {
      setSensorActive(first, true)
    }
  }
}

function setSensorActive(el, active) {
  if (active) {
    el.classList.add("active");
  } else {
    el.classList.remove("active");
  }
}

function liffSendDirectionState(state) {
  console.log("liffSendDirectionState", state)
  if (window.directionCharacteristic) {
    var directionMapping = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06]
    var value = new Uint8Array([directionMapping[state]])
    console.log("liffSendDirectionValue", value)
    window.directionCharacteristic.writeValue(new Uint8Array([directionMapping[state]]))
    .catch(error => {
      uiStatusError(makeErrorMsg("liffSendDirectionState", error), false);
    });
  }
}

function liffSendSpeedState(state) {
  console.log("liffSendSpeedState", state)
  if (window.speedCharacteristic) {
    var speedMapping = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05]
    var value = new Uint8Array([speedMapping[state]])
    console.log("liffSendSpeedValue", value)
    window.speedCharacteristic.writeValue(new Uint8Array([speedMapping[state]]))
    .catch(error => {
      uiStatusError(makeErrorMsg("liffSendSpeedState", error), false);
    });
  }
}

function getJoystickMapping(degree) {
  return parseInt(degree / 60) + 1;
}

function joystickHandler(evt, data) {
  console.log("Force", data.force)
  if (data.force) {
    if (data.force > 1) {
      liffSendSpeedState(5);
    } else if (data.force > 0.8) {
      liffSendSpeedState(4);
    } else if (data.force > 0.7) {
      liffSendSpeedState(3);
    } else if (data.force > 0.6) {
      liffSendSpeedState(2);
    } else if (data.force > 0.5) {
      liffSendSpeedState(1);
    } else {
      liffSendSpeedState(0);
    }
    console.log("Degree", data.angle.degree)
    console.log("JoystickMapping", getJoystickMapping(data.angle.degree))
    liffSendDirectionState(getJoystickMapping(data.angle.degree))
  } else {
    console.log("Reset Speed and Direction")
    liffSendSpeedState(0);
    liffSendDirectionState(0);
  }

}
