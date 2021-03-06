'use strict';

const Vehicle = require('mongoose').model('Vehicle');
const config = require('../../config/config.js');
const {
  sendFeedback
} = require('../../helper');

require('../../helper/stringReplace.js');

/**
 * Diagnostics data required
 * @typedef diagnosticsData
 * @property {string} vehicleName.required - username of requesting vehicle - eg: fota user
 * @property {string} password.required - Password of requesting vehicle - eg: 123
 * @property {string} Diag.required - The diagnostics data - eg: Diagnostics for 123 DTC: 0x50
 */
exports.authenticate_vehicle = function(req, res, next) {

  // fetch user and test password verification
  Vehicle.findOne({
    vehicleName: req.body.vehicleName
  }, function(err, vehicle) {

    if (err || vehicle === null) {
      sendFeedback({
        firmwareID: req.params.firmwareId,
        incrementField: "loginRequest.unknown",
        commandString: "Login attempt.",
        commandOutput: `Vehicle "${req.body.vehicleName}" not found.`,
        res: res,
        sendRequest: "Vehicle not found."
      });

      return;
    }

    vehicle.comparePassword(req.body.password, function(err, isMatch) {

      // Incorrect credentials
      if (err || !isMatch) {

        sendFeedback({
          firmwareID: req.params.firmwareId,
          commandString: "Login attempt.",
          commandOutput: `Incorrect password for vehicle "${req.body.vehicleName}"`,
          res: res,
          sendRequest: "Incorrect password."
        });

        return;
      }

      // ALL OK
      sendFeedback({
        firmwareID: req.params.firmwareId,
        commandString: "Login attempt.",
        commandOutput: `Successfull!"`
      });

      req.hashType = vehicle.hashType;
      next(); // Go to next router in stack
    });
  });

};

/**
 * @route POST /vehicle/diagnostics/{uniqueID}
 * @group Vehicle - End points for vehicle communication
 * @summary Sends diagnostics to the terminal
 * @param {string} uniqueID.path.required - Diagnostics' unique ID - eg: 11122
 * @param {diagnosticsData.model} body.body - Vehicle's diagnostics data
 * @returns {string}  200 - success
 * @returns {Error}  default: Failed parsing diagnostics.
 * @produces text/plain text/html
 */
exports.send_diagnostics = function(req, res) {

  const diagnostics = req.body.Diag.replaceArray(config.DTCs.find, config.DTCs.replace);

  sendFeedback({
    commandString: `Diagnostics firmware with ID ${req.params.uniqueID}`,
    commandOutput: `Firmware deleted successfully!`,
    commandDiagnostics: diagnostics,
    res: res,
    sendRequest: '{"data": "success"}'
  });
}

// // create a user a new user
// var testUser = new Vehicle({
//     vehicleName: "fota user",
//     password: "123",
//     publicKey: `---- BEGIN SSH2 PUBLIC KEY ----
//     Comment: "Car 1"
//     AAAAB3NzaC1yc2EAAAABJQAAAQEAlQYss/H+tKH0xmoes+dZ+P3bqmed2UjAkqk6
//     j/RlZfxbni5qdzRQa+O3+rQEc+yYaCAzwNNVuRjjck3MMDzRyx++2vmY9pGhMRs1
//     asHk277yHrMxbqduEFM9PvFEe7seQ+iEeTgxDOtu9gMIz1pKUeuCAGvXQPdVW/fa
//     LNxeIc2cJzMhVVPyhxzk5ctMcP8+H5Cy2ALhnJZ40c2j2BBFixL+HT1qmXJ0H2k9
//     cbo89jitdiTzLz5w+M8w2KSRMtqPmTYo2oBjlsNvY+QWaUqk036Gohkn/YKjx6nf
//     EdP/XVNr/btQcmPITkJ8XXaFK71uJ2d7hrVG15feNVwD2d6QaQ==
//     ---- END SSH2 PUBLIC KEY ----`
// });
// // save user to database
// testUser.save(function(err) {
//     if (err) throw err;
//   });
