ETW2CTF
=======

ETW to CTF converter

This software allows to convert the event trace format provided by Microsoft in CTF : Common Trace Format.
The output format can be open by Eclipse TMF (Tracing and Monitoring Framework).

=======

To run the software requires the Windows Performance Toolkit.

=======

Instructions:

Generate a Visual Studio project using gyp. <br/>
1. Get gyp using SVN. `svn co http://gyp.googlecode.com/svn/trunk gyp` <br/>
2. Run gyp. `gyp\gyp --depth=. etw2ctf.gyp`

Compile the code with Visual Studio 2010 (or later)

Run the command: etw2ctf.exe ​&lt;tracefile>.etl


=======

Links:

LinuxTools Project (TMF) : http://www.eclipse.org/linuxtools/projectPages/lttng/

Microsoft ETW : http://msdn.microsoft.com/en-us/library/windows/desktop/aa364083%28v=vs.85%29.aspx
