Sound Windows Agent - Release Notes 
=====================================
~~~
Copyright 2025 by Eduard Danziger
~~~

$version$
--------
~~~
Released on $date$
~~~

## Changes
- HttpRequestProcessor renamed to RmqToRestApiForwarder.csproj and moved to the separate repository [rmq-to-rest-api-forwarder](https://github.com/eduarddanziger/rmq-to-rest-api-forwarder.git)

3.3.2
--------
~~~
Released on 01.09.2025
~~~

## New
- CodeSign.cer certificate included, see readme, **SoundDefaultUI**

## Changes
- SoundDefaultUI: Both output and input devices shown; Logging into c:\ProgramData\SoundDefaultUI\

3.3.1
--------
~~~
Released on 20.08.2025
~~~

## New
- Added a simple WPF GUI to display the default sound device, using the Sound Agent C API, implemented in a new SoundAgentApiDll.dll.
- Added possibility to use RabbitMQ as a transport mechanism.

## Changes
- Device properties and events now include the operating system's default render and capture devices.

3.2.5
--------
~~~
Released on 23.07.2025
~~~

## Changes
- Operation system name included into the Rest API
- Discovery, Confirm, and Sound Volume messages
- Latest spdog etc. via vcpkg


3.2.2
--------
~~~
Released on 20.03.2025
~~~

## Changes
- Delivers devices to the Audio Repository Rest API (messaging)
- Separate VolumeRenderChanged and VolumeCaptureChanged events
- README added
--------

3.0.0
--------
~~~
Released on 20.02.2025
~~~

## Changes
- Initial
