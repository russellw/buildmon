# buildmon
Windows build monitor

The problem buildmon was designed to solve (though it can be used for other purposes) is finding out exactly what commands are being issued in the course of building programs on Windows. Thus, it uses Windows event tracing (ETW) to log every process start event with the executable name and command line.

To start tracing, simply run buildmon. To stop tracing, press any key.

The generated log file is called `log.csv`.

A Python script is supplied to extract commands to a batch file in case it is desired to rerun them after modification, though for various reasons, it is not usually possible to repeat a complex build in this way.

ETW can only be accessed from administrator mode, thus buildmon requires elevated privilege.

According to the Microsoft documentation, ETW is only available on Windows Vista or later.
