# Commands

`place order`: Place a buy/sell order.  
`cancel order`: Cancel an existing order.  
`modify order`: Modify an existing order.  
`get positions`: Get all open positions.  
`get orderbook`: Get the orderbook.  
`subscribe`: Subscribe to a public/private channel. Run the `receiver` binary to view subscription messages.

# Build

This project uses `openssl`, `fmt` and `boost` libraries. Make sure you have them installed in your system.

## Installation

```bash
mkdir dist
make
```
