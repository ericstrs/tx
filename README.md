# Transaction tracker

FIFO transaction tracker and portfolio manager.

Accepts CSV file to read in transactions.

* Fees
  * Currently implements the conservative approach: convert transfer fee (in USD) into the transfers corresponding ticker asset, then subtract from transfer asset. That's it. Don't do anything else with the transaction fees.
  * Buying fees are added to cost basis.
  * Selling fees are subtracted from total proceeds once.

## Commands

With environment variables:

* Transaction tracker.
  * `tx log` to be prompted with interface.
  * `tx gen[erate]` to create separate transaction CSV files (buys, sell, etc.).
* Portfolio
  * `tx port[folio]`

**WARNING**: interrupting the interactive prompt will not write any data to the file. To avoid losing large amount of data, you can essential "save" your session by frequently restarting the program.
