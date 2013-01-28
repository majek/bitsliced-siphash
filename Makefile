CFLAGS=-Wall -Wextra -Wno-unused-parameter -msse2 -msse3 -msse4 -msse4.1 -msse4.2 -mmmx -g -O3 -funroll-loops

#CC=gcc -mtune=core2 -march=core2
CC=clang -mavx -maes -mtune=corei7-avx -march=corei7-avx


IN='0\n1\n2\n3\n4\n5\n6\n7\n8\n9\na\nb\nc\nd\ne\nf\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n1a\n1b\n1c\n1d\n1e\n1f\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n2a\n2b\n2c\n2d\n2e\n2f\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n3a\n3b\n3c\n3d\n3e\n3f\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n4a\n4b\n4c\n4d\n4e\n4f\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n5a\n5b\n5c\n5d\n5e\n5f\n60\n61\n62\n63\n64\n65\n66\n67\n68\n69\n6a\n6b\n6c\n6d\n6e\n6f\n70\n71\n72\n73\n74\n75\n76\n77\n78\n79\n7a\n7b\n7c\n7d\n7e\n7f\n80\n81\n82\n83\n84\n85\n86\n87\n88\n89\n8a\n8b\n8c\n8d\n8e\n8f\n90\n91\n92\n93\n94\n95\n96\n97\n98\n99\n9a\n9b\n9c\n9d\n9e\n9f\na0\na1\na2\na3\na4\na5\na6\na7\na8\na9\naa\nab\nac\nad\nae\naf\nb0\nb1\nb2\nb3\nb4\nb5\nb6\nb7\nb8\nb9\nba\nbb\nbc\nbd\nbe\nbf\nc0\nc1\nc2\nc3\nc4\nc5\nc6\nc7\nc8\nc9\nca\ncb\ncc\ncd\nce\ncf\nd0\nd1\nd2\nd3\nd4\nd5\nd6\nd7\nd8\nd9\nda\ndb\ndc\ndd\nde\ndf\ne0\ne1\ne2\ne3\ne4\ne5\ne6\ne7\ne8\ne9\nea\neb\nec\ned\nee\nef\nf0\nf1\nf2\nf3\nf4\nf5\nf6\nf7\nf8\nf9\nfa\nfb\nfc\nfd\nfe\n'
INP=$(IN)$(IN)$(IN)$(IN)

all:
	$(CC) $(CFLAGS) bitslice.c bitmain.c -o bitslice
	$(CC) $(CFLAGS) siphash.c csiphash.c -o siphash
	@echo siphash
	@echo $(INP) | ./siphash > _a
	@echo bitslice
	@echo $(INP) | ./bitslice > _b
	diff _a _b
	@rm _a _b
