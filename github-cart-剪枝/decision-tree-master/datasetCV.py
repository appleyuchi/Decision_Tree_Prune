import pandas as pd
import random as rnd
import math

train_data = pd.read_csv('data/train_dealt.csv')
length = len(train_data)
train_data = train_data.sample(frac=1)
train_data.iloc[0:math.floor(length/3)].to_csv('data/CV-3/train_dealt_1.csv', index=False)
train_data.iloc[math.floor(length/3):math.floor(length/3)*2].to_csv('data/CV-3/train_dealt_2.csv', index=False)
train_data.iloc[math.floor(length/3)*2:length-1].to_csv('data/CV-3/train_dealt_3.csv', index=False)