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
* [] Catch signals to release allocated memory.
* [] Create CSV files for long and short term gains/losses.
* [] Sort the CSV file by date before creating entries CSV file.
* [] Generate buy, sell, transfer, and exchange CSV files with a header.
* [] Simple portfolio metrics file that is suitable for R.
* [] Allow env variables to set path to in and out csv files.
* [] tab completion for options (including when user prompt for `action`).
* [] Encryption for transaction input data.

