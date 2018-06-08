# QtCoroutines
Helper classes for coroutines in Qt.

## Features
- Create and run coroutines from Qt in a platform independent manner
	- Supports Windows and unix
- Manage coroutines with yield, resume, cancel and abort
- Generic await method to create custom awaitables
- Predefined awaitables
	- `timeout`: awaits an internal QTimer that triggers after a specified time
	- `signal`: awaits the emission of a signal, can return the signal arguments
	- `iodevice`: awaits the readready signal of QIODevice and returns the read data
	- `future`: awaits the completition of a QFuture
- Async iterator class with `yield_return` method for easy iteration
- Async Queue class to enqueue to a coroutine that waits for data

## Installation
The package is providet as qpm  package, [`de.skycoder42.qtcoroutines`](https://www.qpm.io/packages/de.skycoder42.qtcoroutines/index.html). You can install it either via qpmx (preferred) or directly via qpm.

### Via qpmx
[qpmx](https://github.com/Skycoder42/qpmx) is a frontend for qpm (and other tools) with additional features, and is the preferred way to install packages. To use it:

1. Install qpmx (See [GitHub - Installation](https://github.com/Skycoder42/qpmx#installation))
2. Install qpm (See [GitHub - Installing](https://github.com/Cutehacks/qpm/blob/master/README.md#installing))
3. In your projects root directory, run `qpmx install de.skycoder42.qtcoroutines`

### Via qpm
1. Install qpm (See [GitHub - Installing](https://github.com/Cutehacks/qpm/blob/master/README.md#installing))
2. In your projects root directory, run `qpm install de.skycoder42.qtcoroutines`
3. Include qpm to your project by adding `include(vendor/vendor.pri)` to your `.pro` file

Check their [GitHub - Usage for App Developers](https://github.com/Cutehacks/qpm/blob/master/README.md#usage-for-app-developers) to learn more about qpm.

## Usage
The general usage is: You create a coroutine from a function and then execute it. Inside the coroutine you can `yield` (or `await`) to return execution back to the caller until the method is resumed (either explicitly or because of an
await condition beeing triggered):

```{.cpp}
void coroutine() {
	qDebug() << "Hello from coroutine";
	QtCoRoutine::yield();
	qDebug() << "Goodbye from coroutine";
}

void coroutine2() {
	qDebug() << "I will continue in 3 seconds";
	QtCoroutine::await(std::chrono::seconds(3));
	qDebug() << "Time to quit!";
	qApp->quit();
}

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);

	auto routine = QtCoroutine::create(coroutine);
	qDebug() << "Hello from main";
	QtCoroutine::resume(routine);
	qDebug() << "Goodbye from main";
	QtCoroutine::resume(routine);

	QtCoroutine::createAndRun(coroutine2);
	qDebug() << "Lets run the app...";
	return app.exec();
}
```

The code above will produce the following output:

```
Hello from main
Hello from coroutine
Goodbye from main
Goodbye from coroutine
I will continue in 3 seconds
Lets run the app...
Time to quit!
```

## References
- https://github.com/tonbit/coroutine
- http://blog.qt.io/blog/2018/05/29/playing-coroutines-qt/