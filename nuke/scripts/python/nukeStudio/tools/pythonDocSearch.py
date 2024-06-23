#   Copyright (c) Chris Bankoff.  All Rights Reserved
#   
#  

def pythonDocSearch():
  import nuke
  import webbrowser
  searchFor = nuke.getInput('Search Python Documentation')
  webSearch = 'http://www.google.com/search?domains=docs.python.org&sitesearch=docs.python.org&sourceid=google-search&q=' + searchFor + '&submit=search'
  webbrowser.open_new(webSearch)
  return
