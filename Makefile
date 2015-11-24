JAVA_HOME = /opt/jdk1.8/
JAVA = java
JAVAC = javac
JAVAC_WITH_PATH = PATH=$(JAVA_HOME)/bin:$(PATH) $(JAVAC)
JAVA_WITH_PATH = PATH=$(JAVA_HOME)/bin:$(PATH) $(JAVA)
TYRUS_LIBS = websocket-ri-archive-1.9/lib
TYRUS_APIS = websocket-ri-archive-1.9/api
TYRUS_EXTS = websocket-ri-archive-1.9/ext
JLIBS = $(TYRUS_APIS)/javax.websocket-api-1.1.jar:$(TYRUS_LIBS)/tyrus-server-1.9.jar:$(TYRUS_LIBS)/tyrus-spi-1.9.jar:$(TYRUS_LIBS)/tyrus-core-1.9.jar:$(TYRUS_LIBS)/tyrus-container-grizzly-server-1.9.jar:$(TYRUS_EXTS)/grizzly-framework-2.3.15-gfa.jar:$(TYRUS_EXTS)/grizzly-http-server-2.3.15-gfa.jar:$(TYRUS_EXTS)/grizzly-http-2.3.15-gfa.jar:$(TYRUS_LIBS)/tyrus-container-grizzly-client-1.9.jar

websocketterm/ReplServer.class: websocketterm/ReplServer.java
	$(JAVAC) websocketterm/ReplServer.java -cp .:$(JLIBS)

websocketterm/WebSocketServer.class: websocketterm/WebSocketServer.java
	$(JAVAC) websocketterm/WebSocketServer.java -cp .:$(JLIBS)

ptyconsole/Util.class: ptyconsole/Util.java
	$(JAVAC) ptyconsole/Util.java

ptyconsole/PTY.class: ptyconsole/PTY.java
	$(JAVAC) ptyconsole/PTY.java

ptyconsole/ptyconsole_PTY.h: ptyconsole/PTY.class
	javah -d ptyconsole ptyconsole.PTY

webterminal/WebTerminal.class: webterminal/WebTerminal.java
	$(JAVAC_WITH_PATH) webterminal/WebTerminal.java

webterminal/WebWriter.class: webterminal/WebWriter.java webterminal/WebTerminal.class
	$(JAVAC_WITH_PATH) webterminal/WebWriter.java

webterminal/ShellConsole.class: webterminal/ShellConsole.java webterminal/WebTerminal.class webterminal/WebWriter.class
	$(JAVAC_WITH_PATH) webterminal/ShellConsole.java

ptyconsole/PtyConsole.class: ptyconsole/PtyConsole.java webterminal/WebTerminal.class webterminal/WebWriter.class ptyconsole/PTY.class
	$(JAVAC_WITH_PATH) ptyconsole/PtyConsole.java

ptyconsole/App.class: ptyconsole/App.java ptyconsole/PtyConsole.class
	$(JAVAC_WITH_PATH) ptyconsole/App.java

libpty.so:
	cd ptyconsole && $(MAKE) all DIST_DIR=.. JDK_HOME=$(JAVA_HOME)

d/domterm: d/domterm.ti
	tic -o. $<

run-pty: ptyconsole/App.class libpty.so d/domterm
	$(JAVA_WITH_PATH) -Djava.library.path=`pwd` ptyconsole.App

run-server: websocketterm/WebSocketServer.class websocketterm/ReplServer.class ptyconsole/Util.class libpty.so d/domterm
	$(JAVA) -cp .:$(JLIBS) -Djava.library.path=`pwd` websocketterm.WebSocketServer

run-shell: webterminal/ShellConsole.class
	$(JAVA_WITH_PATH) webterminal.ShellConsole

clean:
	-rm -rf webterminal/*.class ptyconsole/*.class websocketterm/*.class libpty.so build