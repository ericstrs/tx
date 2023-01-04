# Transaction tracker

FIFO transaction tracker and portfolio manager.

* Fees
  * Takes the conservative approach: convert transfer USD fee into transfers corresponding ticker asset, then subtract from transfer asset. That's it. Don't do anything else with the fees.
  * Buying fees are added to cost basis.
  * Selling fees are subtracted from total proceeds only one time.

## Progress

* [X] combine buy and sell csv file
* [] create csv files for long term and short term gains
* [] catch signals to release allocated memory.

