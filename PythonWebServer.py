#############################################################################
# Program:
#    Lab PythonWebServer, Computer Networks CSE354
# Author:
#    Alex Johnson
# Summary:
#    This program implements a simple python3 web server over TCP sockets.
#############################################################################

import sys
import os
from socket import *

CRLF = '\r\n'


def bindServerSocket():
  """
  Bind a new welcome socket and return the socket and its port number.
  """

  serverPort = int(sys.argv[1]) if len(sys.argv) == 2 else 6789
  serverSocket = socket(AF_INET, SOCK_STREAM)
  serverSocket.bind(('', serverPort))
  return serverSocket, serverPort


def generateResponseString(filename):
  """
  Forms an HTML bytestream response after determining whether the requested file exists.
  """
  # Get the file type of the requested resource
  fileType = determineHtmlContentType(filename)

  # Attempt to open the requested resource and throw an exception if not found.
  fileContents = None
  if fileType == 'text/plain':
    try:
      fileContents = open('.' + filename, 'r').read()

    except FileNotFoundError:
      print('Requested file not found.')
      fileContents = 'Requested file not found.'
  else:
    try:
      with open('.' + filename, 'rb') as f:
        fileContents = f.read()

    except FileNotFoundError:
      print('Requested file not found.')
      fileContents = bytes('Requested file not found.', 'utf-8')

  # Form an HTML response with the file contents if the resource exists
  response = None
  if (fileContents):
    response = 'HTTP/1.0 200 OK' + CRLF + \
        'Content-type: ' + fileType + CRLF + CRLF
  else:
    fileContents = '<HTML><HEAD><TITLE>Not Found</TITLE><BODY>Not Found</BODY></HTML>'
    response = 'HTTP/1.0 404 Not Found' + CRLF + 'Content-type: text/html' + \
        CRLF + CRLF

  # Encode the response as utf-8 to prepare for sending to client
  if fileType == 'text/plain':
    response = bytes(response, 'utf-8') + bytes(fileContents, 'utf-8')
  else:
    response = bytes(response, 'utf-8') + fileContents

  return response


def servePageInfo(connectionSocket):
  """
  The primary functionality of the program. Return the requested content if
  it exists or a 404 if it doesn't.
  """

  # Get the request header, decode it, and parse the request line and requested file
  requestHeader = connectionSocket.recv(1024)
  requestHeader = requestHeader.decode()
  requestLine = requestHeader.split('\n')[0]
  filename = requestLine.split(' ')[1]

  print('\n--------Header Request Line--------\n', requestLine)
  print('\n--------Requested File--------\n', filename)
  print('\n--------Resolving file----------\n')

  response = generateResponseString(filename)
  connectionSocket.send(response)

  return


def determineHtmlContentType(filename):
  """
  Return the filetype of the requested object as an HTML descriptor.
  """

  if filename.endswith('.htm') or filename.endswith('.html'):
    return 'text/html'

  if filename.endswith('.gif'):
    return 'image/gif'

  if filename.endswith('.png'):
    return 'image/png'

  if filename.endswith('.jpg'):
    return 'image/jpeg'

  if filename.endswith('.txt'):
    return 'text/plain'

  return 'application/octet-stream'


def main():
  """
  Drives the program.
  """

  # Start the server by binding a port and listening for requests
  serverSocket, serverPort = bindServerSocket()
  serverSocket.listen(1)
  print('Server listening on port', serverPort)

  try:
    while 1:
      connectionSocket, addr = serverSocket.accept()
      print(addr)
      servePageInfo(connectionSocket)
      connectionSocket.close()

  except KeyboardInterrupt:
    print("\nClosing Server")
    serverSocket.close()
    return


if __name__ == "__main__":
  main()
