# Tx

> [!WARNING]
> This software is a work in progress. Refrain from use in critical environments.

Tx is a tool to locally track your asset transactions using the FIFO (First-In, First-Out) method, making it easier to manage your portfolio and prepare for tax season.

## Features

* Manually log buys, sells, fees and transfers.
* Calculate cost basis and gains/losses.
* Accepts CSV file to read in transactions.
* Format data through CSV file generation for tax preparation.

## Fees

Buying and selling fees are straightforward, but there doesn't seem to be consensus on dealing with transfer fees:

* Buying fees are added to cost basis.
* Selling fees are subtracted from total proceeds once.
* Implements the conservative approach for transfer fees: convert fee (in USD) into the corresponding ticker asset, then subtract from transfer asset. That's it. Don't do anything else with this transaction fees.

## Usage

* `tx <input.csv>` - prompts the logging interface.
* `tx <input.csv> <out.csv>` - generates the formatted CSV file.

**Note**: interrupting the interactive prompt will not write any data to the log. To avoid losing large amount of data, you can "save" your session by periodically restarting the program.
