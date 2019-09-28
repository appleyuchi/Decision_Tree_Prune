import datatable as dt
folder_path = './'
 
 
 
fraud_data= dt.fread(f'{folder_path}train3.data')#读取文件
 
#------------------备份要移动的列到外面-----------------
fraud_data["isFraud"].to_csv("isFraud.csv")
 
 
#-----------删除该列---------------
del fraud_data[:,'isFraud']
 
#-----------追加该列---------------
append_data= dt.fread(f'{folder_path}isFraud.csv')
fraud_data.cbind(append_data)
 
 #---------写入到新的文件---------------
fraud_data.to_csv("train.data")