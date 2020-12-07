import json
import datetime
import pprint
from bson import json_util
from pymongo import MongoClient
import pymongo

#Establish database and collection connection
connection = MongoClient('localhost', 27017)
db = connection['market']
collection = db['stocks']

#insert document function with 3 arguments
def insert_document(document, testquery, query):
  dbConnection = collection.find_one(testquery) #test db connection
  resultCheck = collection.find_one(query) #test query document 
  if dbConnection is None: #output error message if db connection cannot be established
    print "Error 404: database %s not found" % collection 
    return "Try again"
  elif resultCheck is None: #output success message if no document with created ticker exists
    result = collection.insert_one(document) 
    print "Success 200: Document with %s key/pair value created" %query
    return result #perform the insert operation
  else: #output error message if a document with the same generated ticker already exists
    print "Error 404: Document with %s key/pair value already exists" %query
    return "Try again"

#update document function with 5 arguments
def update_document(query, update, stock, old, new):
  result = collection.find_one(query) #test condition for finding the matching document
  if result is None: #if test condition returns None, output error message
    print "Error 404: No document found with Ticker of %s and Volume of %d" % (stock, old)
    return "Try again with different Ticker and Volume values"
  else: #if matching matching document found, perform update operation
    result = collection.update_one(query, update)
    resultQuery = collection.find_one({"Ticker" : stock}, {"Ticker" : 1, "Volume" : 1}) #query to output update result
    print "Success 200: Updated Volume from %d to %d, result shown below" % (old, new)
    return result, resultQuery

#delete document function with query and stock argument
def delete_document(query, stock):
  #setting result to find a document given the query argument
  result = collection.find_one(query)
  if result is None: #if a document does not exist, output erro message
    print "Error 404: No documents found with inputted key/value pair"
    return "Try Again"
  else: #if a document exists, perform the delete operation
    result = collection.delete_one(query)
    print "Success 200: Document with %s ticker deleted" % stock
    return result

#find documents count function with 3 arguments
def find_documents_count(document, low, high):
  result = collection.find_one(document) #test condition for documents
  if result is None: #if no documents found within range, output erro message
    print "Error 404: No documents found between %f and %f values" % (low, high)
    return "Try Again"
  else: #if documents found, perform query and output result
    result = collection.count(document)
    return "Success 200: %d documents found between %f and %f values\n" % (result, low, high)

#industry document query function with 3 arguments
def find_documents_ticker(queryInput, resultOutput, industry):
  result = collection.find_one(queryInput) #test condition for query
  if result is None: #if no documents found, output error message
    print "Error 404: No stocks found within the %s industry" %industry
    return "try again"
  else: #if documents found, perform query and output result
    for result in collection.find(queryInput, resultOutput):
      pprint.pprint(result)
    return "Success 200: Stock tickers within the %s industry are shown above" % (industry)

#aggregate function to get total outstanding shares with 5 arguments
def find_document_total_outstanding_shares(query, MatchArg, SearchArg, SortArg, sector):
  result = collection.find_one(query) #test condition for sector argument
  if result is None: #if no documents found with inputted sector, output error message
    print "Error 404: No documents found in %s Sector" %sector
    return "Try again"
  else: #if documents found, perform aggregate operation
    for result in collection.aggregate([MatchArg, SearchArg, SortArg]): #aggregate operation with match, search and sort argument
      pprint.pprint(result) #print result
    return "Success 200: For the different industries in the %s sector, the total outstanding shares grouped by industries are displayed above" % (sector)

#main method
def main():
	menuChoice = 10
	while menuChoice is not 3:
		menuChoice = input("Enter Your Choice of Action: 1. Manipulation  2. Retrieval  3. Quit    Your Choice: ") # user input stream for initial menuChoice
		#create manipulation logic
		if menuChoice is 1:
			manipulationChoice = input("Enter Your Manipulation Choice: 1. Insert  2. Update  3. Delete    Your Choice: ") #user input stream for manipulation choice
			#create insert logic
			if manipulationChoice is 1:
				#create a new document object with corresponding key/value pairs
  				newStock = {
    				"Ticker" : "ZZ", # generate unique ticker
   					"Profit Margin" : 0.137,
    				"Institutional Ownership" : 0.847,
    				"EPS growth past 5 years" : 0.158,
    				"Total Debt/Equity" : 0.56,
    				"Current Ratio" : 3,
    				"Return on Assets" : 0.089,
    				"Sector" : "Healthcare",
    				"P/S" : 2.54,
    				"Change from Open" : -0.1352,
    				"Performance (YTD)" : 0.2605,
    				"Performance (Week)" : 0.0031,
    				"Quick Ratio" : 2.3,
    				"Insider Transactions" : -0.1352,
    				"P/B" : 3.63,
    				"EPS growth quarter over quarter" : -0.29,
    				"Payout Ratio" : 0.162,
    				"Performance (Quarter)" : 0.0928,
    				"Forward P/E" : 16.11,
    				"P/E" : 19.1,
    				"200-Day Simple Moving Average" : 0.1062,
    				"Shares Outstanding" : 339,
    				"Earnings Date" : datetime.datetime.utcnow(),
    				"52-Week High" : -0.0544,
    				"P/Cash" : 7.45,
    				"Change" : -0.0148,
    				"Analyst Recom" : 1.6,
    				"Volatility (Week)" : 0.0177,
    				"Country" : "USA",
    				"Return on Equity" : 0.182,
    				"50-Day Low" : 0.0728,
    				"Price" : 50.44,
    				"50-Day High" : -0.0544,
    				"Return on Investment" : 0.163,
    				"Shares Float" : 330.21,
    				"Dividend Yield" : 0.0094,
    				"EPS growth next 5 years" : 0.0843,
    				"Industry" : "Medical Laboratories & Research",
    				"Beta" : 1.5,
    				"Sales growth quarter over quarter" : -0.041,
    				"Operating Margin" : 0.187,
    				"EPS (ttm)" : 2.68,
    				"PEG" : 2.27,
    				"Float Short" : 0.008,
    				"52-Week Low" : 0.4378,
    				"Average True Range" : 0.86,
    				"EPS growth next year" : 0.1194,
    				"Sales growth past 5 years" : 0.048,
    				"Company" : "Zed Zepplin Test Inc.",
    				"Gap" : 0,
    				"Relative Volume" : 0.79,
    				"Volatility (Month)" : 0.0168,
    				"Market Cap" : 17356.8,
    				"Volume" : 1847978,
    				"Gross Margin" : 0.512,
    				"Short Ratio" : 1.03,
    				"Performance (Half Year)" : 0.1439,
    				"Relative Strength Index (14)" : 46.51,
    				"Insider Ownership" : 0.001,
    				"20-Day Simple Moving Average" : -0.0172,
    				"Performance (Month)" : 0.0063,
    				"P/Free Cash Flow" : 19.63,
    				"Institutional Transactions" : -0.0074,
    				"Performance (Year)" : 0.4242,
    				"LT Debt/Equity" : 0.56,
    				"Average Volume" : 2569.36,
    				"EPS growth this year" : 0.147,
    				"50-Day Simple Moving Average" : -0.0055
  				}
 				testQueryKeyValue = {"Ticker" : "A"} #test condition for db connection
  				queryKeyValue = {"Ticker" : "ZZ"} #test condition for query 
  				print insert_document(newStock, testQueryKeyValue, queryKeyValue) #call insert document with the 3 corresponding arguments
  			#update logic
  			elif manipulationChoice is 2:
  				Ticker = raw_input("Input a stock Ticker: ") #user input ticker value
  				Volume = input("Input a Volume: ") #user input Volume value
  				newVolume = input("Input a new Volume value to update to the matching document: ") #user input new volume value
  				queryValues = {"Ticker" : Ticker, "Volume" :Volume} #pass in user inputted ticker and volume values to query
  				newValues = {"$set" : {"Volume" : newVolume}} #set user inputted new volume to perform set operation

  				print update_document(queryValues, newValues, Ticker, Volume, newVolume) #call update document function with the 5 corresponding arguments
  			#delete logic
  			elif manipulationChoice is 3:
  				Ticker = raw_input("Input a Stock Ticker: ") #input stream to get user Stock Ticker input
  				queryValues = {"Ticker" : Ticker} #Populating queryValues with user input

  				print delete_document(queryValues, Ticker) #call delete document function
  			#errorhandling for inputted manipulationChoice > 3
  			else:
  				print (str(manipulationChoice) + "is not one of the options.")
  		#retrieval logic
  		elif menuChoice is 2:
  			retrievalChoice = input("Enter Your Retrieval Choice: 1. Number  2. String  3. Aggregation    Your Choice: ")
  			#number retrieval logic
  			if retrievalChoice is 1:
  				lowValue = input("Input a low value for 50-Day Simple Moving Average: ") #user input stream for low value
  				highValue = input("Input a high value for 50-Day Simple Moving Average: ") #user input stream for high value
  				query = {"50-Day Simple Moving Average" : {"$gt": lowValue, "$lt": highValue}} #convert low and high values into query language
  				#call find documents count with corresponding 3 arguments
  				print find_documents_count(query, lowValue, highValue)
  			#string retrieval logic
  			elif retrievalChoice is 2:
  				string = raw_input("Enter a stocks industry: ") #user input stream for industry
  				query = {"Industry" : string} #convert user input to query language
  				output = {"Ticker" : 1} #set query output to ticker
  				#call industry document query function with the 3 corresponding arguments
  				print find_documents_ticker(query, output, string)
  			#aggregation retrieval logic
  			elif retrievalChoice is 3:
  				Sector = raw_input("Input a sector: ") #input stream for user inputted sector
  				testQuery = {"Sector" : Sector} # pass user input to test query
  				aggregateMatchArg = {"$match" : {"Sector" : Sector}} #create match argument with user inputted sector
  				#generate search argument with industry groupings and sum of "Shares Outstanding" from the groups
  				aggregateSearchArg = {"$group" : {"_id" : "$Industry", "Total_Outstanding_Shares" : {"$sum" : "$Shares Outstanding"}}}
  				aggregateSortArg = {"$sort" : {"Total_Outstanding_Shares" : -1}} #generate sort argument, sorting output by Total_Outstanding_Shares
  				#call aggregate function with the 5 corresponding arguments
  				print find_document_total_outstanding_shares(testQuery, aggregateMatchArg, aggregateSearchArg, aggregateSortArg, Sector)
main()