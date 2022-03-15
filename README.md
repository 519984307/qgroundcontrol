# QGroundControl Ground Control Station

## Original Readme
*QGroundControl* (QGC) is an intuitive and powerful ground control station (GCS) for UAVs.

The primary goal of QGC is ease of use for both first time and professional users.
It provides full flight control and mission planning for any MAVLink enabled drone, and vehicle setup for both PX4 and ArduPilot powered UAVs. Instructions for *using QGroundControl* are provided in the [User Manual](https://docs.qgroundcontrol.com/en/) (you may not need them because the UI is very intuitive!)

All the code is open-source, so you can contribute and evolve it as you want.
The [Developer Guide](https://dev.qgroundcontrol.com/en/) explains how to [build](https://dev.qgroundcontrol.com/en/getting_started/) and extend QGC.


Key Links:
* [Website](http://qgroundcontrol.com) (qgroundcontrol.com)
* [User Manual](https://docs.qgroundcontrol.com/en/)
* [Developer Guide](https://dev.qgroundcontrol.com/en/)
* [Discussion/Support](https://docs.qgroundcontrol.com/en/Support/Support.html)
* [Contributing](https://dev.qgroundcontrol.com/en/contribute/)
* [License](https://github.com/mavlink/qgroundcontrol/blob/master/COPYING.md)
---


# __Our__ QGroundControl Ground Control Station
## Fly View
The main page when you open QGC.
### Top Bar
* When no glider is connected, the only other thing in the top bar is the `Start
ROS Menu`. With this you can establish the connection to a glider. Once a glider
is connected, additional items appear in the top bar.
* The `Video Stream Menu` lets you quickly change which video stream is
displayed.
* The `Landing Station Icon` tells you the status of the landing station. If we don't have connection, we can't take off since the belts won't
move.
* The `Stop ROS Button` allows you to stop ROS and even shutdown the Jetson.
### Landing Station Controls
This is a popup that opens when you click on the Landing Station Icon. It
provides buttons to control the belts of the landing station remotely, as well
as extend and retract the hook. It only opens when there is connection to the
landing station.
### Stop ROS Buttons
Those buttons open up, when you click on the Stop ROS Button in the Top Bar.
They allow you to stop ROS, reboot the Jetson and even shutdown the Jetson.
__Shutting down the Jetson means that you physically have to start it up again
on the glider before you can fly again__.  
To make sure that only the connected glider can be shut down, the buttons only
work on the glider you used the Start ROS button on before. If you closed QGC in
the meantime then you're out of luck and there is no way to stop ROS anymore.

## Application Settings
You can access the QGC application settings by clicking on the purple logo in
top left corner and select `Application Settings` in the up-popping popup. Upon
first start, you should verify the following settings are as expected:
* General
   * `Fly View` -> `Video Settings` -> `Source`: "UDP h.264 Video Stream"
   * `Fly View` -> `Video Settings` -> `UDP Port`:
      * 5601 is the main multicopter video stream
      * 5602 is multicopter stream with information about the landing station
      tag detection printed over it
      * 5605 is the main fixed wing video stream
   * `Fly View` -> `Video Settings` -> `Low Latency Mode`: selected
   * `Fly View` -> `Start ROS settings` -> `Jetson Username`: jetsonnx
   * `Fly View` -> `Start ROS settings` -> `... IP address`: Check the IP
   connector website for the IP address of the glider you want to connect to.
* MAVLink
   * `Ground Station` -> `Emit hearbeat`: unselected (unless you fly with the
   telemetry kit (without ROS), then you have to select it).
* Console
   * Here you see the low level consol output of QGC. This can help for
   debugging if something is not working.
## Worklow
### Mission planning
<!-- TODO: Add information what to look out for when planning a mission -->
### Takeoff
1. Start the glider using the `Start ROS` menu in the main toolbar. Select which
Glider you are connected to. (This has to be set before on the IP connector
website).
2. Open plan view with the path icon in the top left corner.
3. Select the file icon right below that and click on `Storage` -> `Open...`.
4. Select the mission file you want to fly.
5. Upload the mission with the pulsating button in the top bar.
6. Make sure the landing station indicator in the top bar (LS
connected/disconnected) is green and shows connected.
7. Go back to Fly view with the fly icon in the top left corner.
8. Move the Joystick in all axes to the full extent and switch through the
flight modes __but don't press the arm button!__
9. Verify the location and orientation shown on the map roughly correspond to
where the glider is in real life.
10. Make sure you are in `Mission` mode (indicate in the top bar on the left).
11. Make sure the takeoff area is free of people, pets and objects that could be
harmed or harm our glider.
12. Arm the glider and it will start to fly the mission automatically.
### Post landing
1. Once the glider makes proper contact with the landing station, disarm it.
2. After disarming, the glider will level itself automatically.
3. Open the landing station control buttons by clicking on the landing station 
connected icon
   * In the `Speed` field, you can set the speed at which the belts move
   * In the `Timeout` field, you set the time for which the belts move after
   pressing one of the buttons once.
   * __!Attention!__ We don't have a way of syncing those values from the
   landing station so to avoid unwanted surprises, select each text field and
   hit enter before starting to control the belts
   * Use the Up, Down, Left, Right and Stop buttons to bring the glider to the
   hooking position. Hook it to the landing station with the Hook buttons
4. Go to the `Log Download` page by selecting the purple icon in the top left
and then clicking on `Analyze Tools` in the popup. The Log Download is the first
tab on this page.
5. Click `Refresh` on the right to get a list of available log files.
6. Select the new log files and click on the `Sharepoint` button on the right.
This will upload them automatically to the right folder in our sharepoint.
   * The more logs there are on the Pixhawk, the longer it takes to refresh
   the list so make sure to `Erase All` from time to time (of course after
   uploading all the logs first).
   * If you don't want to upload them to sharepoint, you can also use the
   `Download` button to download them to your local computer. 
