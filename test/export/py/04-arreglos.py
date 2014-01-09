if __name__ == '__main__':
	a = [str()]*10
	b = [[str()]*30]*20
	c = [str()]*40
	d = [[str()]*5]*50
	n = int(raw_input())
	e = [str()]*n
	a[0] = raw_input()
	a[9] = a[0]
	print(b[9][9])
