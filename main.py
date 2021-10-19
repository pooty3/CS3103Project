import socket
import sys
import threading
import select
import time

class Server:
    def __init__(self, portNumber, telemetryFlag, blacklistArray):
        try:
            self.serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #self.serverSocket.setsockopt((SOL_SOCKET, SOL_REDUSEADDR,1))
        except:
            print("Failed")
            return
        self.serverSocket.bind(('localhost', portNumber))
        self.serverSocket.listen(200)
        self.telemetryFlag = telemetryFlag
        self.blacklistArray = blacklistArray
        self.processSemaphore = threading.Semaphore(8)
        print("Server set up!")

    def startServer(self):
        while True:
            conn, addr = self.serverSocket.accept()
            thread = threading.Thread(name = str(addr), target = self.handleConnection, 
            args = (conn, addr))
            thread.setDaemon(True)
            thread.start()

    def printTelemetry(self, hostname, byteSize, duration):
        print(f'Hostname: {hostname}, Size: {byteSize} bytes, Time: {duration} sec')
        
    def sendBadRequest(self, conn, httpVersion):
        if httpVersion == "HTTP/1.0":
            conn.send(b'HTTP/1.0 400 Bad Request\r\n\r\n')
        else:
            conn.send(b'HTTP/1.1 400 Bad Request\r\n\r\n')
        return

    def sendForbidden(self, conn, httpVersion):
        if httpVersion == "HTTP/1.0":
            conn.send(b'HTTP/1.0 403 Forbidden\r\n\r\n')
        else:
            conn.send(b'HTTP/1.1 403 Forbidden\r\n\r\n')
        return
            
    def handleConnection(self, conn, clientAddr):
        self.processSemaphore.acquire()
        try:
            data = conn.recv(1024)
            if not data:
                return 
            splitdata = data.split(b'\r\n')
            firstLine = splitdata[0].decode("utf-8")
            method, websitetoken, httpVersion = firstLine.split(' ')
            if method != "CONNECT":
                print(f'{method} not supported')
                self.sendBadRequest(conn, httpVersion)
                conn.close()
                self.processSemaphore.release()
                return

            addr,port = websitetoken.split(':')
            blacklisted = any(map(lambda str :  str in addr, self.blacklistArray))
            if blacklisted:
                self.sendForbidden(conn, httpVersion)
                conn.close()
                self.processSemaphore.release()
                return   
            websiteSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            websiteSocket.connect((addr, int(port))) 
        except:
            self.sendBadRequest(conn, httpVersion)
            conn.close()
            self.processSemaphore.release()
            return
        if httpVersion == "HTTP/1.1":
            conn.send(b'HTTP/1.1 200 Connection established\r\n\r\n') 
        elif httpVersion == "HTTP/1.0":
            conn.send(b'HTTP/1.0 200 Connection established\r\n\r\n') 
        else:
            self.sendBadRequest(conn, httpVersion)
            conn.close()
            self.processSemaphore.release()
            return
        toClose = False
        totalBytes = 0
        startTime = time.time()
        while not toClose:
            rlist, _, xlist = select.select([conn, websiteSocket], [], [conn, websiteSocket], 5)
            if xlist or not rlist:
                break
            for r in rlist:
                data = r.recv(8192)
                if not data:
                    toClose = True
                    break
                if r is conn:
                    websiteSocket.sendall(data)
                else:
                    totalBytes += len(data)
                    conn.sendall(data)
        totalTime = time.time() - startTime
        if (self.telemetryFlag == 1):
            self.printTelemetry(addr, totalBytes, totalTime)
        conn.close()
        self.processSemaphore.release()


blacklistArray = []

try:
    blacklist = open(sys.argv[3], "r")
    for line in blacklist:
        blacklistArray.append(line.strip())
    blacklist.close()
except:
    print("No blacklist file found!")
server = Server(int(sys.argv[1]), int(sys.argv[2]), blacklistArray)
server.startServer()
