# Transaction tracker

FIFO transaction tracker and portfolio manager.

Accepts CSV file to read in transactions.

* Fees
  * Currently implements the conservative approach: convert transfer fee (in USD) into the transfers corresponding ticker asset, then subtract from transfer asset. That's it. Don't do anything else with the transaction fees.
  * Buying fees are added to cost basis.
  * Selling fees are subtracted from total proceeds once.

## Progress

* [x] Combine buy and sell CSV files.
* [X] Allow for user interactive prompt and perform validation.
* [X] Create entry field for long or short term gain/loss.
* [] Generate buy, sell, transfer, and exchange CSV files with a header.
* [] Simple portfolio metrics file that is suitable for R.
* [] Tab completion for options (including when user prompt for `action`).
* [] Detect headers for input CSV files.
* [] Allow env variables to set path to in and out CSV files.
* [] Sort the CSV file by date before creating entries CSV file.
* [] Encryption for transaction log file.

**WARNING**: interrupting the interactive prompt will not write any data to the file. To avoid losing large amount of data, you can essential "save" your session by frequently restarting the program.

