import datatable as dt
folder_path = './'
fraud_data= dt.fread(f'{folder_path}train3.data')
 
 
del fraud_data[:,'TransactionID']

 
fraud_data.to_csv("train3.data")