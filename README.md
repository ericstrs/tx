# Transaction tracker

FIFO transaction tracker and portfolio manager.

* Fees
  * Takes the aggressive approach: any gas fees related to investments increase the cost basis of the same asset that was used for gas.
    * What if is USD?
  * Buying fees are added to cost basis.
  * Selling fees are subtracted from total proceeds once.

## Progress

* [X] combine buy and sell csv file
* [] create csv files for long term and short term gains
* [] catch signals to release allocated memory.

