## 1.內容介紹:
c語言寫得cache模擬
功能:
  - 可分別使用:-c、-b、-w(後面接數字)，來設定希望的cache size、one block size、幾ways
  - 傳入檔案:參數-t <<檔案>>可以以R 0X... W 0X...的檔案內容傳入記憶體位置以及做read、write等資訊
  - 總block:cache size/one block size、cache被分成幾sets:總block/幾ways
  - 進而利用重複(>>1)來確認傳入地址的offset、index、tag各佔幾bit
  - 記憶體搬進cache時，利用index來放入cache[index][]，優先放入空白格，若接佔滿則填入上次使用時間最早(離最久)
  - 紀錄hit number:在查找資料時發現已在cache、miss number:在查找時需要從記憶體搬進cache
  - 紀錄dirty bit，標記有做write的檔案被踢出cache時，dram write times增加紀錄共寫了幾次，並在結束時全部檢查誰需要寫入記憶體
  
## 2.測試
模擬(int)C[i][j]=C[i][j]+A[i][k]*B[k][j]，因此需要讀取A[i][k]、B[k][j]、C[i][j]，寫入C[i][j]，並利用三層for迴圈來
測試ijk、ikj順序造成的hit number、miss number變化

迴圈由外到內依序i、j、k:
```
./cache_sim -c 16384 -b 64 -w 4 -t ijk.trace
cache size:16384 block size:64 ways:4 trace file path:ijk.trace
blocks number:256 sets number:64 tag bit length:52 index bit length:6 offset bit length:6
hit number: 1026460 miss number: 22116 dram write times: 256
```

迴圈由外到內依序i、k、j:
```
./cache_sim -c 16384 -b 64 -w 4 -t ikj.trace
cache size:16384 block size:64 ways:4 trace file path:ikj.trace
blocks number:256 sets number:64 tag bit length:52 index bit length:6 offset bit length:6
hit number: 1045801 miss number: 2775 dram write times: 256
```

cache選擇16384剛好是一個A:64*64*4的大小，若選擇48KB會剛好裝下3個陣列根本不用踢出其他人測不出差異，因為offset是0~63
且A[][]大小是4 bytes，代表每16個A[][]就會跳到下一個set，一個A[]就會跳4個set，而miss number會差那麼多，我認為是差在
B，因為A、C是A[i][k]永遠是後面的元素先跳4 bytes在輪的因此可以一直讀到已經放到cache同一個set、block的資料，但至於B[k][j]
，如果是定住j，k在改變，就會變成4個set，4個set在跳，代表都不是讀在同一個block而且B[]每經過16就要輪回一次，當下一輪j改變的
時候就算同一個block的資料有被讀過，但也早已被因為只有4 ways所以被A、C甚至是B自己同set不同block的地址踢出去了

迴圈由外到內依序i、j、k 且 4 ways:
```
./cache_sim -c 16384 -b 64 -w 4 -t ijk.trace
cache size:16384 block size:64 ways:4 trace file path:ijk.trace
blocks number:256 sets number:64 tag bit length:52 index bit length:6 offset bit length:6
hit number: 1026460 miss number: 22116 dram write times: 256
```

迴圈由外到內依序i、j、k 且 1 ways:
```
 ./cache_sim -c 16384 -b 64 -w 1 -t ijk.trace
cache size:16384 block size:64 ways:1 trace file path:ijk.trace
blocks number:256 sets number:256 tag bit length:50 index bit length:8 offset bit length:6
hit number: 909259 miss number: 139317 dram write times: 68797
```

將4 ways換成1 ways之後，miss number從22116增加到139317，A[][]是4 bytes，等於每16格就會換下一個set，A[]每到下一個元素會跳4個set
A剛好一個256 sets的循環，只要set一重複，我認為結果會這樣是因為只有一個way，只要set一重複，就會立刻觸發發剔除重新從記憶體讀去，因此
miss number大幅增加，例如因為C[i][]和A[i][]的i皆相同，所以內圈j、k，k在每16個才跳一次set的情況下，A在每一輪j都會和C發生衝突16次，且每一
次C都會W因此每一次近來馬上寫入之後下一輪A又把他踢走又需要一次dram write，C的元素有64*64再乘以互踢了16次就佔了65536，也在這個反覆互
踢又觸發dirty bit的情況造成dram write time也大幅增加
